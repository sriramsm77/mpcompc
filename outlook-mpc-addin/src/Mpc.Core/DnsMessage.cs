using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;

namespace Mpc.Core
{
    /// <summary>
    /// Minimal DNS wire-format support (RFC 1035) for SRV queries (RFC 2782): just enough to
    /// replace res_nsearch/ns_parserr/dn_expand from srvquery.c without extra dependencies.
    /// </summary>
    public static class DnsMessage
    {
        public const ushort TypeSrv = 33;
        public const ushort ClassIn = 1;

        public const int RcodeNoError = 0;
        public const int RcodeNameError = 3; // NXDOMAIN

        private static readonly IdnMapping Idn = new IdnMapping();

        public static byte[] BuildQuery(ushort id, string name, ushort qtype)
        {
            if (string.IsNullOrEmpty(name)) throw new ArgumentException("A name is required.", "name");

            var buf = new List<byte>(32 + name.Length);
            WriteUInt16(buf, id);
            WriteUInt16(buf, 0x0100); // standard query, recursion desired
            WriteUInt16(buf, 1);      // QDCOUNT
            WriteUInt16(buf, 0);      // ANCOUNT
            WriteUInt16(buf, 0);      // NSCOUNT
            WriteUInt16(buf, 0);      // ARCOUNT

            foreach (string label in name.TrimEnd('.').Split('.'))
            {
                if (label.Length == 0) throw new ArgumentException("Empty label in DNS name: " + name, "name");

                string ascii = IsAscii(label) ? label : Idn.GetAscii(label);
                byte[] bytes = Encoding.ASCII.GetBytes(ascii);
                if (bytes.Length > 63) throw new ArgumentException("DNS label longer than 63 bytes: " + label, "name");

                buf.Add((byte)bytes.Length);
                buf.AddRange(bytes);
            }
            buf.Add(0);

            WriteUInt16(buf, qtype);
            WriteUInt16(buf, ClassIn);

            if (buf.Count > 512) throw new ArgumentException("DNS name too long: " + name, "name");
            return buf.ToArray();
        }

        public static DnsResponse ParseResponse(byte[] message, ushort expectedId)
        {
            if (message == null || message.Length < 12) throw new FormatException("DNS response is too short.");

            int pos = 0;
            ushort id = ReadUInt16(message, ref pos);
            ushort flags = ReadUInt16(message, ref pos);
            int qdCount = ReadUInt16(message, ref pos);
            int anCount = ReadUInt16(message, ref pos);
            ReadUInt16(message, ref pos); // NSCOUNT
            ReadUInt16(message, ref pos); // ARCOUNT

            if (id != expectedId) throw new FormatException("DNS response ID does not match the query.");
            if ((flags & 0x8000) == 0) throw new FormatException("DNS message is not a response.");

            var response = new DnsResponse
            {
                Rcode = flags & 0x000F,
                Truncated = (flags & 0x0200) != 0,
            };

            if (response.Truncated) return response; // caller retries over TCP

            for (int i = 0; i < qdCount; i++)
            {
                ReadName(message, ref pos);
                pos += 4; // QTYPE + QCLASS
                CheckBounds(message, pos, 0);
            }

            for (int i = 0; i < anCount; i++)
            {
                ReadName(message, ref pos);
                ushort type = ReadUInt16(message, ref pos);
                ReadUInt16(message, ref pos); // CLASS
                pos += 4;                     // TTL
                int rdLength = ReadUInt16(message, ref pos);
                CheckBounds(message, pos, rdLength);
                int rdStart = pos;

                if (type == TypeSrv && rdLength >= 7)
                {
                    int p = rdStart;
                    ushort priority = ReadUInt16(message, ref p);
                    ushort weight = ReadUInt16(message, ref p);
                    ushort port = ReadUInt16(message, ref p);
                    string target = ReadName(message, ref p);
                    response.Records.Add(new SrvRecord(priority, weight, port, target));
                }

                pos = rdStart + rdLength;
            }

            return response;
        }

        /// <summary>Reads a possibly compressed name; returns it without the trailing dot.</summary>
        public static string ReadName(byte[] message, ref int pos)
        {
            var labels = new List<string>();
            int cursor = pos;
            bool jumped = false;
            int jumps = 0;

            while (true)
            {
                CheckBounds(message, cursor, 1);
                int len = message[cursor];

                if ((len & 0xC0) == 0xC0)
                {
                    CheckBounds(message, cursor, 2);
                    int pointer = ((len & 0x3F) << 8) | message[cursor + 1];
                    if (!jumped) pos = cursor + 2;
                    jumped = true;
                    if (++jumps > 64) throw new FormatException("DNS name compression loop.");
                    cursor = pointer;
                    continue;
                }

                if ((len & 0xC0) != 0) throw new FormatException("Unsupported DNS label type.");

                cursor++;
                if (len == 0) break;

                CheckBounds(message, cursor, len);
                labels.Add(Encoding.ASCII.GetString(message, cursor, len));
                cursor += len;
            }

            if (!jumped) pos = cursor;
            return string.Join(".", labels);
        }

        private static bool IsAscii(string s)
        {
            foreach (char c in s)
            {
                if (c > 0x7F) return false;
            }
            return true;
        }

        private static void WriteUInt16(List<byte> buf, ushort value)
        {
            buf.Add((byte)(value >> 8));
            buf.Add((byte)(value & 0xFF));
        }

        private static ushort ReadUInt16(byte[] message, ref int pos)
        {
            CheckBounds(message, pos, 2);
            ushort value = (ushort)((message[pos] << 8) | message[pos + 1]);
            pos += 2;
            return value;
        }

        private static void CheckBounds(byte[] message, int pos, int count)
        {
            if (pos < 0 || count < 0 || pos + count > message.Length)
            {
                throw new FormatException("DNS message is truncated or malformed.");
            }
        }
    }

    public sealed class DnsResponse
    {
        public DnsResponse()
        {
            Records = new List<SrvRecord>();
        }

        public int Rcode { get; set; }
        public bool Truncated { get; set; }
        public List<SrvRecord> Records { get; private set; }
    }
}
