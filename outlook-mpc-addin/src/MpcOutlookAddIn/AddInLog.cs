using System;
using System.Globalization;
using System.IO;

namespace MpcOutlookAddIn
{
    /// <summary>Appends diagnostics to %LOCALAPPDATA%\MpcOutlookAddIn\mpc.log.</summary>
    internal static class AddInLog
    {
        private static readonly object Gate = new object();

        public static string LogPath
        {
            get
            {
                return Path.Combine(
                    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                    "MpcOutlookAddIn",
                    "mpc.log");
            }
        }

        public static void Write(string message)
        {
            try
            {
                lock (Gate)
                {
                    string path = LogPath;
                    Directory.CreateDirectory(Path.GetDirectoryName(path));

                    // Keep the log small: start over once it passes 1 MB.
                    var info = new FileInfo(path);
                    if (info.Exists && info.Length > 1024 * 1024) info.Delete();

                    File.AppendAllText(
                        path,
                        DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss", CultureInfo.InvariantCulture) + "  " + message + Environment.NewLine);
                }
            }
            catch (Exception)
            {
                // Logging must never break Outlook.
            }
        }

        public static void Write(string message, Exception ex)
        {
            Write(message + ": " + ex);
        }
    }
}
