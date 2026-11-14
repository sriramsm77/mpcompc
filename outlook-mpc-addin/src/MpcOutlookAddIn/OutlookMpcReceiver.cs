using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using Mpc.Core;
using Outlook = Microsoft.Office.Interop.Outlook;

namespace MpcOutlookAddIn
{
    /// <summary>What one rejoin pass did.</summary>
    internal sealed class RejoinReport
    {
        public int Scanned;
        public int ChunksFound;
        public int Rejoined;
        public int Waiting;
        public int Failed;
        public readonly List<string> WaitingDetails = new List<string>();

        public override string ToString()
        {
            if (ChunksFound == 0) return "No MPC chunks were found in this folder.";

            var lines = new List<string>();
            lines.Add("Rejoined " + Rejoined + " message(s) from " + ChunksFound + " chunk(s).");
            if (Waiting > 0)
            {
                lines.Add(Waiting + " message(s) are still waiting for chunks:");
                lines.AddRange(WaitingDetails.Take(10).Select(d => "  • " + d));
            }
            if (Failed > 0) lines.Add(Failed + " message(s) could not be rejoined; see " + AddInLog.LogPath + ".");
            return string.Join(Environment.NewLine, lines);
        }
    }

    /// <summary>
    /// The receiving side of MPC: port of maildir_mpc_coaelesce_mails_unlocked() in mail.c.
    /// <para>
    /// Chunks are found by reading the X-MPC-* fields from each message's Internet headers and
    /// grouped by X-MPC-ID. When every chunk of a message has arrived, a copy of chunk 1 gets
    /// the joined body, attachments from the other chunks, X-MPC-Chunk "[000/000]" and the
    /// combined X-MPC-Via; the chunks are then moved to Deleted Items.
    /// </para>
    /// </summary>
    /// <summary>
    /// What the scan remembers about a chunk. The message itself is not kept: with IMAP, Outlook
    /// keeps updating unread messages in the background (finishing the download, syncing flags),
    /// and a message object held across that becomes stale; Outlook then refuses to copy it
    /// ("The operation cannot be performed because the message has been changed", 0x80040109).
    /// </summary>
    internal sealed class ChunkRef
    {
        public ChunkRef(string entryId, string subject)
        {
            EntryId = entryId;
            Subject = subject;
        }

        public string EntryId { get; private set; }
        public string Subject { get; private set; }
    }

    internal sealed class OutlookMpcReceiver
    {
        // MAPI_E_OBJECT_CHANGED: the message changed in the mailbox after it was opened.
        private const int ObjectChanged = unchecked((int)0x80040109);
        private const int MaxAttempts = 5;

        private readonly Outlook.Application _application;
        private readonly Func<MpcSettings> _settings;

        public OutlookMpcReceiver(Outlook.Application application, Func<MpcSettings> settings)
        {
            _application = application;
            _settings = settings;
        }

        public RejoinReport RejoinFolder(Outlook.MAPIFolder folder)
        {
            var report = new RejoinReport();
            var chunks = new List<ReceivedChunk<ChunkRef>>();
            string storeId = folder.StoreID;

            Outlook.Items items = folder.Items;
            Outlook.Items scope = Restrict(items);
            try
            {
                foreach (object entry in scope)
                {
                    report.Scanned++;
                    try
                    {
                        var mail = entry as Outlook.MailItem;
                        ReceivedChunk<ChunkRef> chunk = mail == null ? null : ReadChunk(mail);
                        if (chunk != null) chunks.Add(chunk);
                    }
                    finally
                    {
                        OutlookMpc.Release(entry); // only the ID is kept, see ChunkRef
                    }
                }
            }
            finally
            {
                if (!ReferenceEquals(scope, items)) OutlookMpc.Release(scope);
                OutlookMpc.Release(items);
            }

            report.ChunksFound = chunks.Count;

            foreach (ChunkGroup<ChunkRef> group in MpcCoalescer.Group(chunks))
            {
                if (!group.IsComplete)
                {
                    report.Waiting++;
                    report.WaitingDetails.Add(
                        "\"" + group.Chunks[0].Item.Subject + "\": missing chunk " +
                        string.Join(", ", group.MissingChunkNumbers) + " of " + group.Total);
                    continue;
                }

                try
                {
                    Rejoin(group, storeId);
                    report.Rejoined++;
                }
                catch (Exception ex)
                {
                    report.Failed++;
                    AddInLog.Write("Rejoining X-MPC-ID " + group.MpcId + " failed", ex);
                }
            }

            AddInLog.Write("Rejoin pass on \"" + folder.Name + "\": scanned " + report.Scanned + ", chunks " + report.ChunksFound +
                           ", rejoined " + report.Rejoined + ", waiting " + report.Waiting + ", failed " + report.Failed + ".");
            return report;
        }

        private static ReceivedChunk<ChunkRef> ReadChunk(Outlook.MailItem mail)
        {
            if (OutlookMpc.IsRejoined(mail)) return null;

            string headers = OutlookMpc.GetTransportHeaders(mail);
            if (string.IsNullOrEmpty(headers) ||
                headers.IndexOf(MpcConstants.HeaderChunkId, StringComparison.OrdinalIgnoreCase) < 0)
            {
                return null;
            }

            return MpcCoalescer.TryReadChunk(new ChunkRef(mail.EntryID, mail.Subject), headers);
        }

        private void Rejoin(ChunkGroup<ChunkRef> group, string storeId)
        {
            IReadOnlyList<ReceivedChunk<ChunkRef>> parts = group.Chunks;
            string firstId = parts[0].Item.EntryId;

            // Every step opens the chunk fresh from the mailbox, and retries if Outlook reports
            // that it changed in the meantime.
            // Copy() keeps the sender, recipients, dates and folder of chunk 1.
            var merged = WithChunk(firstId, storeId, "copy chunk 1", m => (Outlook.MailItem)m.Copy());
            try
            {
                merged.BodyFormat = Outlook.OlBodyFormat.olFormatPlain;
                merged.Body = MpcCoalescer.MergeBodies(
                    parts.Select(p => WithChunk(p.Item.EntryId, storeId, "read a chunk", m => m.Body ?? string.Empty)).ToList());

                // send.c attaches files to the last chunk; the copy of chunk 1 needs them.
                foreach (ReceivedChunk<ChunkRef> part in parts.Skip(1))
                {
                    WithChunk(part.Item.EntryId, storeId, "copy attachments", m =>
                    {
                        CopyAttachments(m, merged);
                        return true;
                    });
                }

                merged.UnRead = parts.Any(p => WithChunk(p.Item.EntryId, storeId, "read a chunk", m => m.UnRead));

                // As mail.c does: mark the message as rejoined and record every relay.
                string headers = OutlookMpc.GetTransportHeaders(merged)
                    ?? WithChunk(firstId, storeId, "read headers", m => OutlookMpc.GetTransportHeaders(m))
                    ?? string.Empty;
                headers = InternetHeaders.SetValue(headers, MpcConstants.HeaderChunkValue, ChunkValue.Coalesced.ToString());
                headers = InternetHeaders.SetValue(headers, MpcConstants.HeaderVia, group.MergedVia);
                OutlookMpc.TrySetTransportHeaders(merged, headers);
                OutlookMpc.SetInternetHeader(merged, MpcConstants.HeaderChunkValue, ChunkValue.Coalesced.ToString());
                OutlookMpc.SetInternetHeader(merged, MpcConstants.HeaderVia, group.MergedVia);
                OutlookMpc.MarkRejoined(merged);

                merged.Save();
            }
            catch (Exception)
            {
                try
                {
                    merged.Delete();
                }
                catch (Exception ex)
                {
                    AddInLog.Write("Could not remove a partly rejoined message", ex);
                }
                throw;
            }
            finally
            {
                OutlookMpc.Release(merged);
            }

            AddInLog.Write("Rejoined X-MPC-ID " + group.MpcId + " (" + parts.Count + " chunks via " + group.MergedVia + ").");

            if (_settings().DeleteChunksAfterRejoin)
            {
                foreach (ReceivedChunk<ChunkRef> part in parts.Concat(group.Extras))
                {
                    try
                    {
                        WithChunk(part.Item.EntryId, storeId, "delete a chunk", m =>
                        {
                            m.Delete(); // moves to Deleted Items
                            return true;
                        });
                    }
                    catch (Exception ex)
                    {
                        AddInLog.Write("Could not delete an MPC chunk after rejoining", ex);
                    }
                }
            }
        }

        /// <summary>
        /// Opens a chunk fresh from the mailbox and runs <paramref name="action"/> on it. If Outlook
        /// reports that the message changed meanwhile (common for unread IMAP mail), the stale
        /// object is released, and the message is opened again and the action retried.
        /// </summary>
        private T WithChunk<T>(string entryId, string storeId, string what, Func<Outlook.MailItem, T> action)
        {
            for (int attempt = 1; ; attempt++)
            {
                Outlook.MailItem item = Open(entryId, storeId);
                try
                {
                    return action(item);
                }
                catch (COMException ex)
                {
                    if (ex.ErrorCode != ObjectChanged || attempt >= MaxAttempts) throw;
                    AddInLog.Write("Could not " + what + " (the message changed in the mailbox); retrying, attempt " + (attempt + 1) + ".");
                }
                finally
                {
                    OutlookMpc.Release(item);
                }
                Thread.Sleep(250 * attempt);
            }
        }

        private Outlook.MailItem Open(string entryId, string storeId)
        {
            Outlook.NameSpace session = _application.Session;
            try
            {
                var item = session.GetItemFromID(entryId, storeId) as Outlook.MailItem;
                if (item == null) throw new InvalidOperationException("MPC chunk " + entryId + " is no longer in the mailbox.");
                return item;
            }
            finally
            {
                OutlookMpc.Release(session);
            }
        }

        private static void CopyAttachments(Outlook.MailItem from, Outlook.MailItem to)
        {
            Outlook.Attachments source = from.Attachments;
            Outlook.Attachments target = to.Attachments;
            string tempDir = Path.Combine(Path.GetTempPath(), "MpcOutlookAddIn", Guid.NewGuid().ToString("N"));
            try
            {
                int count = source.Count;
                if (count == 0) return;
                Directory.CreateDirectory(tempDir);

                for (int i = 1; i <= count; i++)
                {
                    Outlook.Attachment attachment = source[i];
                    try
                    {
                        string name = SafeFileName(attachment.FileName, i);
                        string path = Path.Combine(tempDir, i.ToString(CultureInfo.InvariantCulture) + "_" + name);
                        attachment.SaveAsFile(path);
                        Outlook.Attachment added = target.Add(path, Outlook.OlAttachmentType.olByValue, Type.Missing, attachment.DisplayName);
                        OutlookMpc.Release(added);
                    }
                    finally
                    {
                        OutlookMpc.Release(attachment);
                    }
                }
            }
            finally
            {
                OutlookMpc.Release(source);
                OutlookMpc.Release(target);
                try
                {
                    if (Directory.Exists(tempDir)) Directory.Delete(tempDir, true);
                }
                catch (IOException)
                {
                    // Outlook may still hold a file; Windows cleans the temp folder eventually.
                }
                catch (UnauthorizedAccessException)
                {
                }
            }
        }

        private static string SafeFileName(string name, int index)
        {
            if (string.IsNullOrWhiteSpace(name)) return "attachment" + index;
            foreach (char c in Path.GetInvalidFileNameChars()) name = name.Replace(c, '_');
            return name;
        }

        /// <summary>Limits the scan to recently received messages (RejoinLookbackDays).</summary>
        private Outlook.Items Restrict(Outlook.Items items)
        {
            int days = _settings().RejoinLookbackDays;
            if (days <= 0) return items;

            // Outlook's [ReceivedTime] filter expects a date in the user's locale format.
            string since = DateTime.Now.AddDays(-days).ToString("g", CultureInfo.CurrentCulture);
            try
            {
                return items.Restrict("[ReceivedTime] >= '" + since + "'");
            }
            catch (Exception ex)
            {
                AddInLog.Write("Could not filter by received date (" + since + "); scanning the whole folder", ex);
                return items;
            }
        }
    }
}
