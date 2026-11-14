using System;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Mpc.Core
{
    /// <summary>
    /// The value of the X-MPC-Chunk header: "[n/m]" where n is this chunk (1-based) and m is the
    /// total number of chunks. "[000/000]" marks a message that has already been rejoined.
    /// </summary>
    public struct ChunkValue : IEquatable<ChunkValue>
    {
        private static readonly Regex Pattern =
            new Regex(@"^\[\s*(\d{1,3})\s*/\s*(\d{1,3})\s*\]", RegexOptions.CultureInvariant);

        public static readonly ChunkValue Coalesced = new ChunkValue(0, 0);

        public ChunkValue(int index, int total)
        {
            if (index < 0) throw new ArgumentOutOfRangeException("index");
            if (total < 0) throw new ArgumentOutOfRangeException("total");
            Index = index;
            Total = total;
        }

        public int Index { get; private set; }

        public int Total { get; private set; }

        /// <summary>True for "[000/000]", which mail.c writes on a rejoined message.</summary>
        public bool IsCoalesced
        {
            get { return Index == 0 && Total == 0; }
        }

        /// <summary>True when this describes one chunk of a split message.</summary>
        public bool IsValidChunk
        {
            get { return Total >= 1 && Index >= 1 && Index <= Total; }
        }

        /// <summary>Formats as "[%03d/%03d]", matching sprintf in send.c and mail.c.</summary>
        public override string ToString()
        {
            return string.Format(CultureInfo.InvariantCulture, "[{0:D3}/{1:D3}]", Index, Total);
        }

        public static bool TryParse(string text, out ChunkValue value)
        {
            value = default(ChunkValue);
            if (text == null) return false;

            Match m = Pattern.Match(text.Trim());
            if (!m.Success) return false;

            value = new ChunkValue(
                int.Parse(m.Groups[1].Value, CultureInfo.InvariantCulture),
                int.Parse(m.Groups[2].Value, CultureInfo.InvariantCulture));
            return true;
        }

        public bool Equals(ChunkValue other)
        {
            return Index == other.Index && Total == other.Total;
        }

        public override bool Equals(object obj)
        {
            return obj is ChunkValue && Equals((ChunkValue)obj);
        }

        public override int GetHashCode()
        {
            return (Index * 397) ^ Total;
        }
    }
}
