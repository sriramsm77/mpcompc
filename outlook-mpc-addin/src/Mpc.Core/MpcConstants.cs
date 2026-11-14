using System.Globalization;

namespace Mpc.Core
{
    /// <summary>
    /// Constants shared with the mailutils implementation
    /// (include/mailutils/header.h and include/mailutils/mpc.h).
    /// </summary>
    public static class MpcConstants
    {
        /// <summary>X-MPC-Via: the host named in the SRV record that relays this chunk.</summary>
        public const string HeaderVia = "X-MPC-Via";

        /// <summary>X-MPC-ID: a UUID shared by every chunk of one message.</summary>
        public const string HeaderChunkId = "X-MPC-ID";

        /// <summary>X-MPC-Chunk: "[n/m]", formatted as "[%03d/%03d]".</summary>
        public const string HeaderChunkValue = "X-MPC-Chunk";

        /// <summary>X-MPC-Result: placeholder for a computation result; sent blank.</summary>
        public const string HeaderResult = "X-MPC-Result";

        /// <summary>Value the sender puts in X-MPC-Result (mailutils sends a single space).</summary>
        public const string BlankResultValue = " ";

        /// <summary>MAX_MPC_SERVERS in mpc.h: the most SRV records (and so chunks) used.</summary>
        public const int MaxMpcServers = 2;

        /// <summary>
        /// Builds the SRV name for the n-th MPC server. As in send.c, numbering starts at 1:
        /// _mpc1._tcp.example.com, _mpc2._tcp.example.com, ...
        /// </summary>
        public static string SrvName(int index, string domain)
        {
            return string.Format(CultureInfo.InvariantCulture, "_mpc{0}._tcp.{1}", index, domain);
        }
    }
}
