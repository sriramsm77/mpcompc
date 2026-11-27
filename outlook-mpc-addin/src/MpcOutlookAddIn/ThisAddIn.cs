using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using Mpc.Core;
using Office = Microsoft.Office.Core;
using Outlook = Microsoft.Office.Interop.Outlook;

namespace MpcOutlookAddIn
{
    public partial class ThisAddIn
    {
        private System.Windows.Forms.Timer _rejoinTimer;

        /// <summary>
        /// The Items collection of every account's Inbox, watched for new mail. Outlook's NewMailEx
        /// event isn't raised reliably for IMAP accounts (such as the lab's alice and bob), so each
        /// Inbox is watched as well. The references are kept so the events stay connected.
        /// </summary>
        private readonly List<Outlook.Items> _watchedInboxes = new List<Outlook.Items>();

        /// <summary>Stores already logged as having no Inbox, so the log isn't repeated each pass.</summary>
        private readonly HashSet<string> _storesWithoutInbox = new HashSet<string>();

        private MpcSettings _settings;

        /// <summary>
        /// Loaded on first use, because Outlook builds the main window's ribbon (and calls
        /// MpcRibbon.GetPressed, which reads these settings) before the Startup event runs.
        /// </summary>
        internal MpcSettings Settings
        {
            get
            {
                if (_settings == null) _settings = LoadSettings();
                return _settings;
            }
        }

        internal OutlookMpcSender Sender { get; private set; }
        internal OutlookMpcReceiver Receiver { get; private set; }

        /// <summary>%APPDATA%\MpcOutlookAddIn\settings.ini</summary>
        internal static string SettingsPath
        {
            get
            {
                return Path.Combine(
                    Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                    "MpcOutlookAddIn",
                    "settings.ini");
            }
        }

        private static MpcSettings LoadSettings()
        {
            try
            {
                return MpcSettings.Load(SettingsPath);
            }
            catch (Exception ex)
            {
                AddInLog.Write("Could not read " + SettingsPath + "; using defaults", ex);
                return new MpcSettings();
            }
        }

        private void ThisAddIn_Startup(object sender, EventArgs e)
        {
            Sender = new OutlookMpcSender(Application, () => Settings, CreateResolver);
            Receiver = new OutlookMpcReceiver(Application, () => Settings);

            Application.ItemSend += new Outlook.ApplicationEvents_11_ItemSendEventHandler(Application_ItemSend);
            Application.NewMailEx += new Outlook.ApplicationEvents_11_NewMailExEventHandler(Application_NewMailEx);

            // Rejoin passes run on Outlook's UI thread, shortly after the trigger, and batch up
            // bursts of new mail into one pass.
            _rejoinTimer = new System.Windows.Forms.Timer { Interval = 3000 };
            _rejoinTimer.Tick += RejoinTimer_Tick;

            AddInLog.Write("MPC add-in started. Split on send: " + Settings.AutoSplitOnSend +
                           ", rejoin new mail: " + Settings.AutoRejoinNewMail + ".");

            WatchInboxes();

            // mail.c rejoins chunks when the mailbox is opened; do the same for the Inboxes.
            if (Settings.AutoRejoinNewMail) ScheduleInboxRejoin();
        }

        private void ThisAddIn_Shutdown(object sender, EventArgs e)
        {
            // Outlook does not raise Shutdown reliably for add-ins; nothing here needs it.
            if (_rejoinTimer != null) _rejoinTimer.Dispose();
        }

        protected override Office.IRibbonExtensibility CreateRibbonExtensibilityObject()
        {
            return new MpcRibbon();
        }

        internal void SaveSettings()
        {
            try
            {
                Settings.Save(SettingsPath);
            }
            catch (Exception ex)
            {
                AddInLog.Write("Could not save " + SettingsPath, ex);
            }
        }

        /// <summary>Re-reads settings.ini so edits made in Notepad aren't overwritten.</summary>
        internal void ReloadSettings()
        {
            try
            {
                _settings = MpcSettings.Load(SettingsPath);
            }
            catch (Exception ex)
            {
                AddInLog.Write("Could not re-read " + SettingsPath + "; keeping the current settings", ex);
            }
        }

        internal void ScheduleInboxRejoin()
        {
            if (_rejoinTimer == null) return;
            _rejoinTimer.Stop();
            _rejoinTimer.Start();
        }

        private ISrvResolver CreateResolver()
        {
            IList<IPEndPoint> servers = null;
            try
            {
                servers = DnsSrvResolver.ParseServerList(Settings.DnsServers);
            }
            catch (FormatException ex)
            {
                AddInLog.Write("Ignoring DnsServers in " + SettingsPath + "; using this computer's DNS servers", ex);
            }

            return new DnsSrvResolver(servers != null && servers.Count > 0 ? servers : null, Settings.DnsTimeoutMs);
        }

        private void Application_ItemSend(object item, ref bool cancel)
        {
            Sender.OnItemSend(item, ref cancel);
        }

        private void Application_NewMailEx(string entryIdCollection)
        {
            if (Settings.AutoRejoinNewMail) ScheduleInboxRejoin();
        }

        private void Inbox_ItemAdd(object item)
        {
            if (Settings.AutoRejoinNewMail) ScheduleInboxRejoin();
        }

        /// <summary>Connects <see cref="Inbox_ItemAdd"/> to the Inbox of every account open at startup.</summary>
        private void WatchInboxes()
        {
            ForEachInbox((inbox, storeName) =>
            {
                Outlook.Items items = inbox.Items;
                items.ItemAdd += new Outlook.ItemsEvents_ItemAddEventHandler(Inbox_ItemAdd);
                _watchedInboxes.Add(items);
                AddInLog.Write("Watching the Inbox of " + storeName + " for new mail.");
            });
        }

        /// <summary>
        /// Rejoins the Inbox of every account in the profile, not just the default one: with
        /// alice's and bob's accounts in one profile, only the first one's Inbox is the default.
        /// </summary>
        private void RejoinTimer_Tick(object sender, EventArgs e)
        {
            _rejoinTimer.Stop();
            if (!Settings.AutoRejoinNewMail) return;

            ForEachInbox((inbox, storeName) => Receiver.RejoinFolder(inbox));
        }

        /// <summary>
        /// Runs <paramref name="action"/> on the Inbox of each store (account) in the profile. A
        /// store without an Inbox (a PST archive, say) is skipped; an error in one store doesn't stop
        /// the others.
        /// </summary>
        private void ForEachInbox(Action<Outlook.MAPIFolder, string> action)
        {
            Outlook.NameSpace session = null;
            Outlook.Stores stores = null;
            try
            {
                session = Application.Session;
                stores = session.Stores;
                for (int i = 1; i <= stores.Count; i++)
                {
                    Outlook.Store store = null;
                    Outlook.MAPIFolder inbox = null;
                    string storeName = "store " + i;
                    try
                    {
                        store = stores[i];
                        storeName = store.DisplayName;
                        try
                        {
                            inbox = store.GetDefaultFolder(Outlook.OlDefaultFolders.olFolderInbox);
                        }
                        catch (Exception ex)
                        {
                            if (_storesWithoutInbox.Add(storeName)) AddInLog.Write("Skipping " + storeName + ": it has no Inbox", ex);
                            continue;
                        }

                        action(inbox, storeName);
                    }
                    catch (Exception ex)
                    {
                        AddInLog.Write("Automatic rejoin: the Inbox of " + storeName + " failed", ex);
                    }
                    finally
                    {
                        OutlookMpc.Release(inbox);
                        OutlookMpc.Release(store);
                    }
                }
            }
            catch (Exception ex)
            {
                AddInLog.Write("Automatic rejoin: could not list the accounts", ex);
            }
            finally
            {
                OutlookMpc.Release(stores);
                OutlookMpc.Release(session);
            }
        }

        #region VSTO generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InternalStartup()
        {
            this.Startup += new System.EventHandler(ThisAddIn_Startup);
            this.Shutdown += new System.EventHandler(ThisAddIn_Shutdown);
        }

        #endregion
    }
}
