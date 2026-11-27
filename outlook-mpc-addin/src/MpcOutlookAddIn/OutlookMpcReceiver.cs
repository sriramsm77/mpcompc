using System;
using System.Collections.Generic;
using System.Globalization;
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
        public int FailedSenderChecks;
        public int SuspiciousDuplicates;
        public int SkippedFlagged;
        public readonly List<string> WaitingDetails = new List<string>();
        public readonly List<string> SenderCheckDetails = new List<string>();
        public readonly List<string> SuspiciousDetails = new List<string>();

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
            if (FailedSenderChecks > 0)
            {
                lines.Add(FailedSenderChecks + " message(s) failed the sender checks and were not rejoined. Their chunks were moved to Junk Email, in the category \"" +
                          OutlookMpcReceiver.SuspiciousCategory + "\", and won't be listed again (remove the category to check them again):");
                lines.AddRange(SenderCheckDetails.Take(10).Select(d => "  • " + d));
            }
            if (SuspiciousDuplicates > 0)
            {
                lines.Add(SuspiciousDuplicates + " duplicate chunk(s) failed the sender checks; they were moved to Junk Email, in the category \"" +
                          OutlookMpcReceiver.SuspiciousCategory + "\":");
                lines.AddRange(SuspiciousDetails.Take(10).Select(d => "  • " + d));
            }
            if (Failed > 0) lines.Add(Failed + " message(s) could not be rejoined; see " + AddInLog.LogPath + ".");
            return string.Join(Environment.NewLine, lines);
        }
    }

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

    /// <summary>
    /// The receiving side of MPC, based on maildir_mpc_coaelesce_mails_unlocked() in mail.c.
    /// <para>
    /// Pieces are found by reading the X-MPC-* fields from each message's Internet headers and
    /// grouped by X-MPC-ID. When every piece of a message has arrived, a copy of piece 1 gets the
    /// rebuilt content, X-MPC-Chunk "[000/000]" and the combined X-MPC-Via; the pieces are then
    /// marked as rejoined and moved to Deleted Items. The content is rebuilt in one of two ways:
    /// </para>
    /// <list type="bullet">
    /// <item>X-MPC-Scheme "additive-p61" (sent by this add-in): the .bin share files of every
    /// piece are added up mod p to rebuild the body and the attachments.</item>
    /// <item>No X-MPC-Scheme (sent by mailutils): the body slices are joined in order and the
    /// attachments copied from the other pieces.</item>
    /// </list>
    /// <para>
    /// Every step opens the piece fresh from the mailbox (see <see cref="ChunkRef"/>).
    /// </para>
    /// </summary>
    internal sealed class OutlookMpcReceiver
    {
        // MAPI_E_OBJECT_CHANGED: the message changed in the mailbox after it was opened.
        private const int ObjectChanged = unchecked((int)0x80040109);
        private const int MaxAttempts = 5;

        /// <summary>Outlook category given to duplicate chunks that failed the sender checks.</summary>
        internal const string SuspiciousCategory = "MPC: failed sender checks";

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
                        if (mail != null && HasCategory(mail.Categories, SuspiciousCategory)) report.SkippedFlagged++;
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

            foreach (ChunkGroup<ChunkRef> found in MpcCoalescer.Group(chunks))
            {
                ChunkGroup<ChunkRef> group = found;
                if (!group.IsComplete)
                {
                    report.Waiting++;
                    report.WaitingDetails.Add(
                        "\"" + group.Chunks[0].Item.Subject + "\": missing chunk " +
                        string.Join(", ", group.MissingChunkNumbers) + " of " + group.Total);
                    continue;
                }

                // Stage 1 (sender checks): every chunk must have passed SPF/DKIM/DMARC at the relay
                // named in its X-MPC-Via. The chunks are left as they are, so a message rejoins once
                // the setting is turned off, and they show up in every report until then.
                // Every copy of a chunk is checked (a replayed or forged duplicate can carry the same
                // X-MPC-ID): a passing copy is used where there is one, and copies that fail are kept
                // and reported, not deleted with the rest.
                SenderCheckSelection<ChunkRef> selection = null;
                if (_settings().RequireSenderChecks)
                {
                    selection = SenderChecks.SelectPassing(group);
                    group = selection.Group;
                    SenderCheckResult check = SenderChecks.Evaluate(group);
                    if (!check.Passed)
                    {
                        report.FailedSenderChecks++;
                        report.SenderCheckDetails.Add("\"" + group.Chunks[0].Item.Subject + "\": " + check.Problems[0]);
                        AddInLog.Write("Not rejoining X-MPC-ID " + group.MpcId + ": sender checks failed: " + string.Join("; ", check.Problems));
                        FlagFailed(found, storeId);
                        continue;
                    }
                }

                try
                {
                    Rejoin(group, storeId);
                    report.Rejoined++;
                    if (selection != null) KeepSuspicious(selection, storeId, report);
                }
                catch (Exception ex)
                {
                    report.Failed++;
                    AddInLog.Write("Rejoining X-MPC-ID " + group.MpcId + " failed", ex);
                }
            }

            AddInLog.Write("Rejoin pass on \"" + folder.Name + "\": scanned " + report.Scanned + ", chunks " + report.ChunksFound +
                           ", rejoined " + report.Rejoined + ", waiting " + report.Waiting + ", failed sender checks " + report.FailedSenderChecks +
                           ", suspicious duplicates " + report.SuspiciousDuplicates +
                           ", skipped (already failed) " + report.SkippedFlagged +
                           ", failed " + report.Failed + ".");
            return report;
        }

        private static ReceivedChunk<ChunkRef> ReadChunk(Outlook.MailItem mail)
        {
            if (OutlookMpc.IsRejoined(mail)) return null;
            // Already failed the sender checks: dealt with, until the user removes the category.
            if (HasCategory(mail.Categories, SuspiciousCategory)) return null;

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

            if (!group.HasOneScheme)
            {
                throw new InvalidOperationException("Its pieces have different X-MPC-Scheme values.");
            }

            // For shares, rebuild everything before touching the mailbox: if the shares don't
            // add up, nothing is created and the pieces stay where they are.
            SharedMessage rebuilt = null;
            if (group.IsAdditiveShares)
            {
                var filesPerShare = parts
                    .Select(p => WithChunk(p.Item.EntryId, storeId, "read share files",
                        m => (IReadOnlyList<MailFile>)OutlookMpc.ReadAttachments(m, SharedMessage.IsShareFileName)))
                    .ToList();
                rebuilt = SharedMessage.Combine(filesPerShare);
            }
            else if (group.Scheme != null)
            {
                throw new NotSupportedException("Unknown X-MPC-Scheme \"" + group.Scheme + "\".");
            }

            // Every step opens the piece fresh from the mailbox, and retries if Outlook reports
            // that it changed in the meantime.
            // Copy() keeps the sender, recipients, dates and folder of piece 1.
            var merged = WithChunk(firstId, storeId, "copy piece 1", m => (Outlook.MailItem)m.Copy());
            try
            {
                merged.BodyFormat = Outlook.OlBodyFormat.olFormatPlain;

                if (rebuilt != null)
                {
                    // The copy carries piece 1's share files; replace them with the real files.
                    OutlookMpc.RemoveAttachmentsFrom(merged, 1);
                    merged.Body = rebuilt.Body;
                    OutlookMpc.AddAttachments(merged, rebuilt.Attachments);
                }
                else
                {
                    merged.Body = MpcCoalescer.MergeBodies(
                        parts.Select(p => WithChunk(p.Item.EntryId, storeId, "read a chunk", m => m.Body ?? string.Empty)).ToList());

                    // send.c attaches files to the last chunk; the copy of chunk 1 needs them.
                    foreach (ReceivedChunk<ChunkRef> part in parts.Skip(1))
                    {
                        List<MailFile> files = WithChunk(part.Item.EntryId, storeId, "read attachments", m => OutlookMpc.ReadAttachments(m, null));
                        OutlookMpc.AddAttachments(merged, files);
                    }
                }

                merged.UnRead = parts.Any(p => WithChunk(p.Item.EntryId, storeId, "read a piece", m => m.UnRead));

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

            AddInLog.Write("Rejoined X-MPC-ID " + group.MpcId + " (" + parts.Count + (rebuilt != null ? " shares" : " chunks") +
                           " via " + group.MergedVia + ").");

            bool delete = _settings().DeleteChunksAfterRejoin;
            foreach (ReceivedChunk<ChunkRef> part in parts.Concat(group.Extras))
            {
                try
                {
                    WithChunk(part.Item.EntryId, storeId, delete ? "mark and delete a piece" : "mark a piece", m =>
                    {
                        // Mark every piece so a later pass never rebuilds this message again,
                        // even when the pieces are kept (DeleteChunksAfterRejoin=false).
                        OutlookMpc.MarkRejoined(m);
                        m.Save();

                        if (delete) m.Delete(); // moves to Deleted Items
                        return true;
                    });
                }
                catch (Exception ex)
                {
                    AddInLog.Write("Could not mark or delete an MPC piece after rejoining", ex);
                }
            }
        }

        /// <summary>
        /// After a message was rejoined, its duplicate chunks that failed the sender checks are kept
        /// for the user to see: they get the <see cref="SuspiciousCategory"/> category and the
        /// rejoined marker (so later passes don't keep listing them as waiting), are moved to the
        /// account's Junk Email folder, and are reported.
        /// </summary>
        private void KeepSuspicious(SenderCheckSelection<ChunkRef> selection, string storeId, RejoinReport report)
        {
            for (int i = 0; i < selection.Suspicious.Count; i++)
            {
                ReceivedChunk<ChunkRef> copy = selection.Suspicious[i];
                string detail = "\"" + copy.Item.Subject + "\": " + selection.SuspiciousDetails[i];
                report.SuspiciousDuplicates++;
                report.SuspiciousDetails.Add(detail);
                AddInLog.Write("Kept a suspicious duplicate of X-MPC-ID " + selection.Group.MpcId + ": " + selection.SuspiciousDetails[i]);
                try
                {
                    WithChunk(copy.Item.EntryId, storeId, "mark a suspicious duplicate", m =>
                    {
                        OutlookMpc.MarkRejoined(m);
                        m.Categories = AddCategory(m.Categories, SuspiciousCategory);
                        m.Save();
                        return MoveToJunk(m);
                    });
                }
                catch (Exception ex)
                {
                    AddInLog.Write("Could not mark a suspicious duplicate", ex);
                }
            }
        }

        /// <summary>
        /// Gives every chunk of a message that failed the sender checks (and any copies of them) the
        /// <see cref="SuspiciousCategory"/> category. Chunks with it are skipped from then on, so the
        /// message is reported once; removing the category makes the add-in check it again. The
        /// chunks are moved to the account's Junk Email folder (kept, not deleted) and aren't marked
        /// rejoined.
        /// </summary>
        private void FlagFailed(ChunkGroup<ChunkRef> group, string storeId)
        {
            foreach (ReceivedChunk<ChunkRef> part in group.Chunks.Concat(group.Extras))
            {
                try
                {
                    WithChunk(part.Item.EntryId, storeId, "mark a chunk that failed the sender checks", m =>
                    {
                        m.Categories = AddCategory(m.Categories, SuspiciousCategory);
                        m.Save();
                        return MoveToJunk(m);
                    });
                }
                catch (Exception ex)
                {
                    AddInLog.Write("Could not mark a chunk that failed the sender checks", ex);
                }
            }
        }

        /// <summary>Folder names tried when an account has no default Junk Email folder.</summary>
        private static readonly string[] JunkFolderNames = { "Junk Email", "Junk E-mail", "Junk", "Spam", "Junk Mail" };

        /// <summary>
        /// Moves a chunk to its account's Junk Email folder, like a mail server's quarantine: out of
        /// the Inbox, but kept for inspection. If the account has none, the chunk stays where it is
        /// (it still has the category). <paramref name="mail"/> can't be used afterwards.
        /// </summary>
        private static bool MoveToJunk(Outlook.MailItem mail)
        {
            Outlook.MAPIFolder current = null;
            Outlook.Store store = null;
            Outlook.MAPIFolder junk = null;
            object moved = null;
            try
            {
                current = mail.Parent as Outlook.MAPIFolder;
                if (current == null) return false;
                store = current.Store;
                junk = FindJunkFolder(store);
                if (junk == null)
                {
                    AddInLog.Write("No Junk Email folder in " + store.DisplayName + "; the chunk stays in " + current.Name + ".");
                    return false;
                }
                if (string.Equals(junk.EntryID, current.EntryID, StringComparison.Ordinal)) return true;

                moved = mail.Move(junk);
                return true;
            }
            finally
            {
                OutlookMpc.Release(moved);
                OutlookMpc.Release(junk);
                OutlookMpc.Release(store);
                OutlookMpc.Release(current);
            }
        }

        private static Outlook.MAPIFolder FindJunkFolder(Outlook.Store store)
        {
            try
            {
                return store.GetDefaultFolder(Outlook.OlDefaultFolders.olFolderJunk);
            }
            catch (COMException)
            {
                // Some accounts (IMAP, say) have no default Junk folder; look for one by name.
            }

            Outlook.MAPIFolder root = null;
            Outlook.Folders folders = null;
            try
            {
                root = store.GetRootFolder();
                folders = root.Folders;
                for (int i = 1; i <= folders.Count; i++)
                {
                    Outlook.MAPIFolder folder = folders[i];
                    if (JunkFolderNames.Contains(folder.Name, StringComparer.OrdinalIgnoreCase)) return folder;
                    OutlookMpc.Release(folder);
                }
            }
            catch (COMException ex)
            {
                AddInLog.Write("Could not look for a Junk folder in " + store.DisplayName, ex);
            }
            finally
            {
                OutlookMpc.Release(folders);
                OutlookMpc.Release(root);
            }
            return null;
        }

        /// <summary>True if Outlook's comma-separated category list contains <paramref name="category"/>.</summary>
        internal static bool HasCategory(string categories, string category)
        {
            return (categories ?? string.Empty)
                .Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
                .Any(c => string.Equals(c.Trim(), category, StringComparison.OrdinalIgnoreCase));
        }

        /// <summary>Adds <paramref name="category"/> to Outlook's comma-separated category list.</summary>
        internal static string AddCategory(string categories, string category)
        {
            List<string> list = (categories ?? string.Empty)
                .Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
                .Select(c => c.Trim())
                .Where(c => c.Length > 0)
                .ToList();
            if (!list.Contains(category, StringComparer.OrdinalIgnoreCase)) list.Add(category);
            return string.Join(", ", list);
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
