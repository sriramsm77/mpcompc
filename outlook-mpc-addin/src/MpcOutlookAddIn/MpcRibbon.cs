using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Office = Microsoft.Office.Core;
using Outlook = Microsoft.Office.Interop.Outlook;

namespace MpcOutlookAddIn
{
    /// <summary>
    /// Ribbon XML callbacks. The main window gets an "MPC" group on the Home tab
    /// (MpcRibbon.Explorer.xml); a new message gets a "Send via MPC" button on the
    /// Message tab (MpcRibbon.Compose.xml).
    /// </summary>
    [ComVisible(true)]
    public class MpcRibbon : Office.IRibbonExtensibility
    {
        private const string Caption = "MPC";

        public string GetCustomUI(string ribbonID)
        {
            switch (ribbonID)
            {
                case "Microsoft.Outlook.Explorer":
                    return GetResourceText("MpcRibbon.Explorer.xml");
                case "Microsoft.Outlook.Mail.Compose":
                    return GetResourceText("MpcRibbon.Compose.xml");
                default:
                    return null;
            }
        }

        public void OnSendViaMpc(Office.IRibbonControl control)
        {
            var inspector = control.Context as Outlook.Inspector;
            var mail = inspector == null ? null : inspector.CurrentItem as Outlook.MailItem;
            if (mail == null)
            {
                MessageBox.Show("Open a new email message to send it via MPC.", Caption, MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            Globals.ThisAddIn.Sender.SendFromRibbon(mail);
        }

        public void OnRejoinFolder(Office.IRibbonControl control)
        {
            var explorer = control.Context as Outlook.Explorer;
            Outlook.MAPIFolder folder = explorer == null ? null : explorer.CurrentFolder;
            if (folder == null) return;

            Cursor previous = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            try
            {
                RejoinReport report = Globals.ThisAddIn.Receiver.RejoinFolder(folder);
                Cursor.Current = previous;
                MessageBox.Show(report.ToString(), Caption, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                Cursor.Current = previous;
                AddInLog.Write("Rejoin from the ribbon failed", ex);
                MessageBox.Show("Rejoining MPC chunks failed:\n\n" + ex.Message, Caption, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public bool GetPressed(Office.IRibbonControl control)
        {
            switch (control.Id)
            {
                case "MpcAutoSplitToggle":
                    return Globals.ThisAddIn.Settings.AutoSplitOnSend;
                case "MpcAutoRejoinToggle":
                    return Globals.ThisAddIn.Settings.AutoRejoinNewMail;
                default:
                    return false;
            }
        }

        public void OnToggle(Office.IRibbonControl control, bool pressed)
        {
            switch (control.Id)
            {
                case "MpcAutoSplitToggle":
                    Globals.ThisAddIn.Settings.AutoSplitOnSend = pressed;
                    break;
                case "MpcAutoRejoinToggle":
                    Globals.ThisAddIn.Settings.AutoRejoinNewMail = pressed;
                    if (pressed) Globals.ThisAddIn.ScheduleInboxRejoin();
                    break;
                default:
                    return;
            }
            Globals.ThisAddIn.SaveSettings();
        }

        public void OnOpenSettings(Office.IRibbonControl control)
        {
            try
            {
                Globals.ThisAddIn.SaveSettings(); // make sure the file exists
                Process.Start("notepad.exe", "\"" + ThisAddIn.SettingsPath + "\"");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Could not open " + ThisAddIn.SettingsPath + ":\n\n" + ex.Message, Caption, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private static string GetResourceText(string fileName)
        {
            Assembly assembly = Assembly.GetExecutingAssembly();
            string resource = assembly.GetManifestResourceNames()
                .FirstOrDefault(n => n.EndsWith("." + fileName, StringComparison.OrdinalIgnoreCase));
            if (resource == null) return null;

            using (Stream stream = assembly.GetManifestResourceStream(resource))
            using (var reader = new StreamReader(stream))
            {
                return reader.ReadToEnd();
            }
        }
    }
}
