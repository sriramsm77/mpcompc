using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace Mpc.Core
{
    /// <summary>Add-in options, stored as a simple key=value file.</summary>
    public sealed class MpcSettings
    {
        public MpcSettings()
        {
            AutoSplitOnSend = true;
            AutoRejoinNewMail = true;
            DeleteChunksAfterRejoin = true;
            RejoinLookbackDays = 30;
            DnsServers = string.Empty;
            DnsTimeoutMs = 2000;
        }

        /// <summary>Split every outgoing message whose recipient domain publishes _mpcN SRV records.</summary>
        public bool AutoSplitOnSend { get; set; }

        /// <summary>Rejoin chunks in the Inbox at startup and whenever new mail arrives.</summary>
        public bool AutoRejoinNewMail { get; set; }

        /// <summary>Move chunks to Deleted Items once they are rejoined (mail.c deletes them).</summary>
        public bool DeleteChunksAfterRejoin { get; set; }

        /// <summary>Only look at messages received in the last N days (0 = all).</summary>
        public int RejoinLookbackDays { get; set; }

        /// <summary>Comma-separated DNS servers for SRV lookups; empty = this computer's DNS servers.</summary>
        public string DnsServers { get; set; }

        public int DnsTimeoutMs { get; set; }

        public static MpcSettings Load(string path)
        {
            var settings = new MpcSettings();
            if (string.IsNullOrEmpty(path) || !File.Exists(path)) return settings;

            foreach (string rawLine in File.ReadAllLines(path, Encoding.UTF8))
            {
                string line = rawLine.Trim();
                if (line.Length == 0 || line.StartsWith("#", StringComparison.Ordinal) || line.StartsWith(";", StringComparison.Ordinal)) continue;

                int eq = line.IndexOf('=');
                if (eq <= 0) continue;

                settings.Apply(line.Substring(0, eq).Trim(), line.Substring(eq + 1).Trim());
            }

            return settings;
        }

        public void Save(string path)
        {
            string dir = Path.GetDirectoryName(path);
            if (!string.IsNullOrEmpty(dir)) Directory.CreateDirectory(dir);

            var lines = new List<string>
            {
                "# MPC Outlook add-in settings",
                "AutoSplitOnSend=" + Format(AutoSplitOnSend),
                "AutoRejoinNewMail=" + Format(AutoRejoinNewMail),
                "DeleteChunksAfterRejoin=" + Format(DeleteChunksAfterRejoin),
                "RejoinLookbackDays=" + RejoinLookbackDays.ToString(CultureInfo.InvariantCulture),
                "# Comma-separated DNS servers for _mpcN._tcp SRV lookups; empty uses this computer's DNS",
                "DnsServers=" + (DnsServers ?? string.Empty),
                "DnsTimeoutMs=" + DnsTimeoutMs.ToString(CultureInfo.InvariantCulture),
            };
            File.WriteAllLines(path, lines.ToArray(), new UTF8Encoding(false));
        }

        private void Apply(string key, string value)
        {
            switch (key.ToLowerInvariant())
            {
                case "autosplitonsend":
                    AutoSplitOnSend = ParseBool(value, AutoSplitOnSend);
                    break;
                case "autorejoinnewmail":
                    AutoRejoinNewMail = ParseBool(value, AutoRejoinNewMail);
                    break;
                case "deletechunksafterrejoin":
                    DeleteChunksAfterRejoin = ParseBool(value, DeleteChunksAfterRejoin);
                    break;
                case "rejoinlookbackdays":
                    RejoinLookbackDays = Math.Max(0, ParseInt(value, RejoinLookbackDays));
                    break;
                case "dnsservers":
                    DnsServers = value;
                    break;
                case "dnstimeoutms":
                    DnsTimeoutMs = Math.Max(100, ParseInt(value, DnsTimeoutMs));
                    break;
            }
        }

        private static string Format(bool value)
        {
            return value ? "true" : "false";
        }

        private static bool ParseBool(string value, bool fallback)
        {
            switch (value.Trim().ToLowerInvariant())
            {
                case "true":
                case "yes":
                case "1":
                case "on":
                    return true;
                case "false":
                case "no":
                case "0":
                case "off":
                    return false;
                default:
                    return fallback;
            }
        }

        private static int ParseInt(string value, int fallback)
        {
            int result;
            return int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out result) ? result : fallback;
        }
    }
}
