using System;
using System.Collections.Generic;

namespace Mpc.Core
{
    /// <summary>One message to send: a slice of the body plus its X-MPC-* headers.</summary>
    public sealed class OutgoingChunk
    {
        public OutgoingChunk(ChunkValue chunk, string body, string via, string mpcId)
        {
            Chunk = chunk;
            Body = body;
            Via = via;
            MpcId = mpcId;

            // Same headers, values and order as mpc_mail_compose_send__() in send.c.
            Headers = new[]
            {
                new KeyValuePair<string, string>(MpcConstants.HeaderVia, via),
                new KeyValuePair<string, string>(MpcConstants.HeaderChunkId, mpcId),
                new KeyValuePair<string, string>(MpcConstants.HeaderChunkValue, chunk.ToString()),
                new KeyValuePair<string, string>(MpcConstants.HeaderResult, MpcConstants.BlankResultValue),
            };
        }

        public ChunkValue Chunk { get; private set; }
        public string Body { get; private set; }
        public string Via { get; private set; }
        public string MpcId { get; private set; }

        /// <summary>As in send.c, attachments travel only with the last chunk.</summary>
        public bool CarriesAttachments
        {
            get { return Chunk.Index == Chunk.Total; }
        }

        public IReadOnlyList<KeyValuePair<string, string>> Headers { get; private set; }
    }

    public static class MpcSendPlanner
    {
        /// <summary>
        /// A new X-MPC-ID. send.c uses uuid_generate_random + uuid_unparse_upper, which gives
        /// the same upper-case 8-4-4-4-12 text as this.
        /// </summary>
        public static string NewMpcId()
        {
            return Guid.NewGuid().ToString("D").ToUpperInvariant();
        }

        /// <summary>
        /// Splits <paramref name="body"/> into one chunk per server and pairs each chunk with
        /// the server that relays it (chunk n goes via the n-th SRV target).
        /// </summary>
        public static IReadOnlyList<OutgoingChunk> Plan(string body, IReadOnlyList<SrvRecord> servers, string mpcId)
        {
            if (servers == null || servers.Count == 0) throw new ArgumentException("At least one MPC server is required.", "servers");
            if (string.IsNullOrWhiteSpace(mpcId)) throw new ArgumentException("An MPC ID is required.", "mpcId");

            IReadOnlyList<string> parts = BodyChunker.Split(body, servers.Count);
            var chunks = new List<OutgoingChunk>(parts.Count);
            for (int i = 0; i < parts.Count; i++)
            {
                chunks.Add(new OutgoingChunk(
                    new ChunkValue(i + 1, parts.Count),
                    parts[i],
                    servers[i].Target.TrimEnd('.'),
                    mpcId));
            }
            return chunks;
        }
    }
}
