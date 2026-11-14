using System.Collections.Generic;
using System.Linq;

namespace Mpc.Core
{
    public static class MpcServerDiscovery
    {
        /// <summary>
        /// Port of getSRVRecords() in send.c: looks up _mpc1._tcp.&lt;domain&gt;,
        /// _mpc2._tcp.&lt;domain&gt;, ... and stops at the first name without an SRV record.
        /// The number of servers found is the number of chunks to send.
        /// </summary>
        public static IReadOnlyList<SrvRecord> DiscoverServers(
            ISrvResolver resolver, string domain, int maxServers = MpcConstants.MaxMpcServers)
        {
            var servers = new List<SrvRecord>();
            if (resolver == null || string.IsNullOrWhiteSpace(domain)) return servers;

            for (int i = 1; i <= maxServers; i++)
            {
                SrvRecord best = PickRecord(resolver.Resolve(MpcConstants.SrvName(i, domain.Trim())));
                if (best == null) break;
                servers.Add(best);
            }

            return servers;
        }

        /// <summary>
        /// resolveSRV() uses the first SRV answer. When a name has several, this prefers the
        /// lowest priority and otherwise keeps DNS order. A target of "." means "no service".
        /// </summary>
        public static SrvRecord PickRecord(IReadOnlyList<SrvRecord> records)
        {
            if (records == null) return null;
            return records
                .Where(r => !string.IsNullOrEmpty(r.Target) && r.Target != ".")
                .OrderBy(r => r.Priority)
                .FirstOrDefault();
        }
    }
}
