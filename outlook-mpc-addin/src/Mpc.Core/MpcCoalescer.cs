using System;
using System.Collections.Generic;
using System.Linq;

namespace Mpc.Core
{
    /// <summary>One received chunk, tied to whatever object holds the message (T).</summary>
    public sealed class ReceivedChunk<T>
    {
        public ReceivedChunk(T item, string mpcId, ChunkValue chunk, string via)
        {
            Item = item;
            MpcId = mpcId;
            Chunk = chunk;
            Via = via ?? string.Empty;
        }

        public T Item { get; private set; }
        public string MpcId { get; private set; }
        public ChunkValue Chunk { get; private set; }
        public string Via { get; private set; }
    }

    /// <summary>All the chunks that share one X-MPC-ID.</summary>
    public sealed class ChunkGroup<T>
    {
        internal ChunkGroup(string mpcId, int total, List<ReceivedChunk<T>> ordered, List<ReceivedChunk<T>> extras)
        {
            MpcId = mpcId;
            Total = total;
            Chunks = ordered;
            Extras = extras;
        }

        public string MpcId { get; private set; }

        /// <summary>The "m" in [n/m].</summary>
        public int Total { get; private set; }

        /// <summary>One chunk per index, in chunk order.</summary>
        public IReadOnlyList<ReceivedChunk<T>> Chunks { get; private set; }

        /// <summary>Second copies of a chunk index, or chunks whose total disagrees.</summary>
        public IReadOnlyList<ReceivedChunk<T>> Extras { get; private set; }

        /// <summary>True when chunks 1..Total have all arrived.</summary>
        public bool IsComplete
        {
            get { return Total >= 1 && Chunks.Count == Total; }
        }

        /// <summary>
        /// X-MPC-Via for the rejoined message: each chunk's Via joined with ", " in chunk order,
        /// as maildir_mpc_coaelesce_mails_unlocked() builds it.
        /// </summary>
        public string MergedVia
        {
            get { return string.Join(", ", Chunks.Select(c => c.Via)); }
        }

        public IEnumerable<int> MissingChunkNumbers
        {
            get
            {
                var have = new HashSet<int>(Chunks.Select(c => c.Chunk.Index));
                return Enumerable.Range(1, Math.Max(0, Total)).Where(i => !have.Contains(i));
            }
        }
    }

    /// <summary>
    /// Port of _mpc_get_chunk_info() and maildir_mpc_coaelesce_mails_unlocked() in mail.c.
    /// </summary>
    public static class MpcCoalescer
    {
        /// <summary>
        /// Reads the X-MPC-* headers from a raw header block. Returns null when the message is
        /// not an MPC chunk: no X-MPC-ID, a rejoined message ("[000/000]"), an unreadable
        /// X-MPC-Chunk, or no X-MPC-Via (mail.c treats the last case as an error and skips it).
        /// </summary>
        public static ReceivedChunk<T> TryReadChunk<T>(T item, string rawHeaders)
        {
            string id = InternetHeaders.GetFirst(rawHeaders, MpcConstants.HeaderChunkId);
            if (string.IsNullOrWhiteSpace(id)) return null;

            ChunkValue chunk;
            if (!ChunkValue.TryParse(InternetHeaders.GetFirst(rawHeaders, MpcConstants.HeaderChunkValue), out chunk)) return null;
            if (!chunk.IsValidChunk) return null; // includes [000/000]

            string via = InternetHeaders.GetFirst(rawHeaders, MpcConstants.HeaderVia);
            if (via == null) return null;

            return new ReceivedChunk<T>(item, id.Trim(), chunk, via);
        }

        /// <summary>
        /// Groups chunks by X-MPC-ID and sorts each group by chunk number. Unlike mail.c, a group
        /// is reported as incomplete until every chunk has arrived, so a message is never
        /// rejoined with pieces missing.
        /// </summary>
        public static IReadOnlyList<ChunkGroup<T>> Group<T>(IEnumerable<ReceivedChunk<T>> chunks)
        {
            var groups = new List<ChunkGroup<T>>();
            if (chunks == null) return groups;

            foreach (IGrouping<string, ReceivedChunk<T>> byId in chunks
                .Where(c => c != null)
                .GroupBy(c => c.MpcId, StringComparer.OrdinalIgnoreCase))
            {
                List<ReceivedChunk<T>> all = byId.ToList();

                // The total most chunks agree on (normally they all agree).
                int total = all
                    .GroupBy(c => c.Chunk.Total)
                    .OrderByDescending(g => g.Count())
                    .ThenByDescending(g => g.Key)
                    .First().Key;

                var ordered = new List<ReceivedChunk<T>>();
                var extras = new List<ReceivedChunk<T>>();
                var seen = new HashSet<int>();

                foreach (ReceivedChunk<T> c in all.OrderBy(c => c.Chunk.Index))
                {
                    if (c.Chunk.Total != total || !seen.Add(c.Chunk.Index))
                    {
                        extras.Add(c);
                    }
                    else
                    {
                        ordered.Add(c);
                    }
                }

                groups.Add(new ChunkGroup<T>(all[0].MpcId, total, ordered, extras));
            }

            return groups;
        }

        /// <summary>Joins chunk bodies in order (mu_stream_copy of each chunk onto the first).</summary>
        public static string MergeBodies(IEnumerable<string> bodiesInChunkOrder)
        {
            return string.Concat(bodiesInChunkOrder ?? Enumerable.Empty<string>());
        }
    }
}
