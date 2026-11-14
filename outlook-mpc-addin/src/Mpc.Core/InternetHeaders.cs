using System;
using System.Collections.Generic;
using System.Text;

namespace Mpc.Core
{
    /// <summary>
    /// Reads and edits a raw RFC 5322 header block, such as the one Outlook keeps in
    /// PR_TRANSPORT_MESSAGE_HEADERS for a received message.
    /// </summary>
    public static class InternetHeaders
    {
        /// <summary>Header fields in order, with folded lines joined and values trimmed.</summary>
        public static IReadOnlyList<KeyValuePair<string, string>> Parse(string raw)
        {
            var fields = new List<KeyValuePair<string, string>>();
            if (string.IsNullOrEmpty(raw)) return fields;

            string name = null;
            StringBuilder value = null;

            foreach (string line in SplitLines(raw))
            {
                if (line.Length == 0) break; // end of the header block

                if ((line[0] == ' ' || line[0] == '\t') && name != null)
                {
                    value.Append(' ').Append(line.Trim());
                    continue;
                }

                if (name != null) fields.Add(new KeyValuePair<string, string>(name, value.ToString().Trim()));
                name = null;

                int colon = line.IndexOf(':');
                if (colon <= 0) continue; // not a header field; ignore it

                name = line.Substring(0, colon).Trim();
                value = new StringBuilder(line.Substring(colon + 1).Trim());
            }

            if (name != null) fields.Add(new KeyValuePair<string, string>(name, value.ToString().Trim()));
            return fields;
        }

        /// <summary>
        /// Value of the first field called <paramref name="name"/> (any case), or null.
        /// Like mu_header_aget_value_n(hdr, name, 1, ...) in mail.c.
        /// </summary>
        public static string GetFirst(string raw, string name)
        {
            foreach (KeyValuePair<string, string> field in Parse(raw))
            {
                if (string.Equals(field.Key, name, StringComparison.OrdinalIgnoreCase)) return field.Value;
            }
            return null;
        }

        /// <summary>
        /// Replaces the first field called <paramref name="name"/> (and its folded lines) with
        /// "name: value", or adds it at the end of the header block if it is missing, like
        /// mu_header_set_value(hdr, name, value, 1).
        /// </summary>
        public static string SetValue(string raw, string name, string value)
        {
            string newline = (raw != null && raw.IndexOf("\r\n", StringComparison.Ordinal) >= 0) ? "\r\n" : "\n";
            string newField = name + ": " + value;

            List<string> lines = SplitLines(raw ?? string.Empty);

            // Index of the blank line that ends the header block (or the end of the text).
            int end = lines.FindIndex(l => l.Length == 0);
            if (end < 0) end = lines.Count;

            for (int i = 0; i < end; i++)
            {
                string line = lines[i];
                if (line.Length == 0 || line[0] == ' ' || line[0] == '\t') continue;

                int colon = line.IndexOf(':');
                if (colon <= 0) continue;
                if (!string.Equals(line.Substring(0, colon).Trim(), name, StringComparison.OrdinalIgnoreCase)) continue;

                int next = i + 1;
                while (next < end && lines[next].Length > 0 && (lines[next][0] == ' ' || lines[next][0] == '\t')) next++;

                lines.RemoveRange(i, next - i);
                lines.Insert(i, newField);
                return Join(lines, newline, raw);
            }

            // Not found: add it at the end of the header block (before any blank line).
            int insertAt = end;
            while (insertAt > 0 && lines[insertAt - 1].Length == 0) insertAt--;
            lines.Insert(insertAt, newField);
            return Join(lines, newline, raw);
        }

        private static List<string> SplitLines(string raw)
        {
            return new List<string>(raw.Replace("\r\n", "\n").Split('\n'));
        }

        private static string Join(List<string> lines, string newline, string original)
        {
            string joined = string.Join(newline, lines);
            bool hadTrailingNewline = !string.IsNullOrEmpty(original) && original.EndsWith("\n", StringComparison.Ordinal);
            if (hadTrailingNewline && !joined.EndsWith(newline, StringComparison.Ordinal)) joined += newline;
            return joined;
        }
    }
}
