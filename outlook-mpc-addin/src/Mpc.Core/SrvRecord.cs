using System;
using System.Collections.Generic;

namespace Mpc.Core
{
    /// <summary>One DNS SRV answer (the SRVRecord struct in srvquery.h).</summary>
    public sealed class SrvRecord
    {
        public SrvRecord(ushort priority, ushort weight, ushort port, string target)
        {
            Priority = priority;
            Weight = weight;
            Port = port;
            Target = target ?? string.Empty;
        }

        public ushort Priority { get; private set; }
        public ushort Weight { get; private set; }
        public ushort Port { get; private set; }

        /// <summary>Target host name, without the trailing dot.</summary>
        public string Target { get; private set; }

        public override string ToString()
        {
            return Priority + " " + Weight + " " + Port + " " + Target;
        }
    }

    /// <summary>Looks up SRV records for one fully qualified name.</summary>
    public interface ISrvResolver
    {
        /// <summary>
        /// Returns the SRV answers for <paramref name="name"/>; an empty list means the name
        /// has no SRV records. Throws <see cref="DnsLookupException"/> when the lookup itself
        /// failed (no DNS server answered, or the server reported an error).
        /// </summary>
        IReadOnlyList<SrvRecord> Resolve(string name);
    }

    public sealed class DnsLookupException : Exception
    {
        public DnsLookupException(string message) : base(message) { }
        public DnsLookupException(string message, Exception inner) : base(message, inner) { }
    }
}
