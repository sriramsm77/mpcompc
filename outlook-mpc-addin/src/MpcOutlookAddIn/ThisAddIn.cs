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
            Receiver = new OutlookMpcReceiver(() => Settings);

            Application.ItemSend += new Outlook.ApplicationEvents_11_ItemSendEventHandler(Application_ItemSend);
            Application.NewMailEx += new Outlook.ApplicationEvents_11_NewMailExEventHandler(Application_NewMailEx);

            // Rejoin passes run on Outlook's UI thread, shortly after the trigger, and batch up
            // bursts of new mail into one pass.
            _rejoinTimer = new System.Windows.Forms.Timer { Interval = 3000 };
            _rejoinTimer.Tick += RejoinTimer_Tick;

            AddInLog.Write("MPC add-in started. Split on send: " + Settings.AutoSplitOnSend +
                           ", rejoin new mail: " + Settings.AutoRejoinNewMail + ".");

            // mail.c rejoins chunks when the mailbox is opened; do the same for the Inbox.
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

        private void RejoinTimer_Tick(object sender, EventArgs e)
        {
            _rejoinTimer.Stop();
            if (!Settings.AutoRejoinNewMail) return;

            Outlook.NameSpace session = null;
            Outlook.MAPIFolder inbox = null;
            try
            {
                session = Application.Session;
                inbox = session.GetDefaultFolder(Outlook.OlDefaultFolders.olFolderInbox);
                Receiver.RejoinFolder(inbox);
            }
            catch (Exception ex)
            {
                AddInLog.Write("Automatic rejoin of the Inbox failed", ex);
            }
            finally
            {
                OutlookMpc.Release(inbox);
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
