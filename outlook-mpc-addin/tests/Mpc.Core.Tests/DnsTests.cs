using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace Mpc.Core.Tests
{
    public static class DnsTests
    {
        [Test]
        public static void BuildsAStandardSrvQuery()
        {
            byte[] q = DnsMessage.BuildQuery(0xBEEF, "_mpc1._tcp.example.com", DnsMessage.TypeSrv);
            Assert.Equal(0xBE, (int)q[0]);
            Assert.Equal(0xEF, (int)q[1]);
            Assert.Equal(0x01, (int)q[2], "RD flag");
            Assert.Equal(1, (int)q[5], "one question");
            int pos = 12;
            Assert.Equal("_mpc1._tcp.example.com", DnsMessage.ReadName(q, ref pos));
            Assert.Equal(33, (q[pos] << 8) | q[pos + 1], "QTYPE SRV");
            Assert.Equal(1, (q[pos + 2] << 8) | q[pos + 3], "QCLASS IN");
        }

        [Test]
        public static void ParsesCompressedSrvAnswers()
        {
            byte[] query = DnsMessage.BuildQuery(7, "_mpc1._tcp.example.com", DnsMessage.TypeSrv);
            byte[] response = FakeDns.Answer(query, 0, false, new[]
            {
                new SrvRecord(10, 5, 2525, "relay1.example.net"),
                new SrvRecord(20, 0, 25, "relay1.example.net"), // FakeDns compresses this one
            });

            DnsResponse r = DnsMessage.ParseResponse(response, 7);
            Assert.Equal(0, r.Rcode);
            Assert.Equal(2, r.Records.Count, "records");
            Assert.Equal("relay1.example.net", r.Records[0].Target);
            Assert.Equal((ushort)2525, r.Records[0].Port);
            Assert.Equal((ushort)10, r.Records[0].Priority);
            Assert.Equal((ushort)5, r.Records[0].Weight);
            Assert.Equal("relay1.example.net", r.Records[1].Target, "compressed target");

            Assert.Throws<FormatException>(() => DnsMessage.ParseResponse(response, 8), "wrong ID");
            Assert.Throws<FormatException>(() => DnsMessage.ParseResponse(response.Take(response.Length - 3).ToArray(), 7), "truncated bytes");
        }

        [Test]
        public static void RejectsCompressionLoops()
        {
            var msg = new byte[] { 0xC0, 0x00 };
            int pos = 0;
            Assert.Throws<FormatException>(() => DnsMessage.ReadName(msg, ref pos), "loop");
        }

        [Test]
        public static void ResolvesOverUdpEndToEnd()
        {
            using (var dns = new FakeDns())
            {
                dns.Zone["_mpc1._tcp.example.com"] = new[] { new SrvRecord(0, 0, 25, "relay1.example.net") };
                dns.Zone["_mpc2._tcp.example.com"] = new[] { new SrvRecord(0, 0, 25, "relay2.example.org") };

                var resolver = new DnsSrvResolver(new[] { dns.EndPoint }, 1000, 1);
                IReadOnlyList<SrvRecord> servers = MpcServerDiscovery.DiscoverServers(resolver, "example.com");
                Assert.Sequence(new[] { "relay1.example.net", "relay2.example.org" }, servers.Select(s => s.Target), "servers");

                Assert.Equal(0, resolver.Resolve("_mpc1._tcp.nowhere.test").Count, "NXDOMAIN is no record");
            }
        }

        [Test]
        public static void FallsBackToTcpWhenTruncated()
        {
            using (var dns = new FakeDns { TruncateUdp = true })
            {
                dns.Zone["_mpc1._tcp.example.com"] = new[] { new SrvRecord(0, 0, 25, "relay-over-tcp.example.net") };
                var resolver = new DnsSrvResolver(new[] { dns.EndPoint }, 1000, 1);
                Assert.Equal("relay-over-tcp.example.net", resolver.Resolve("_mpc1._tcp.example.com").Single().Target);
                Assert.True(dns.TcpQueries > 0, "used TCP");
            }
        }

        [Test]
        public static void ReportsServerFailuresAndTimeouts()
        {
            using (var dns = new FakeDns { Rcode = 2 }) // SERVFAIL
            {
                var resolver = new DnsSrvResolver(new[] { dns.EndPoint }, 500, 1);
                Assert.Throws<DnsLookupException>(() => resolver.Resolve("_mpc1._tcp.example.com"), "SERVFAIL");
            }

            // A UDP port nobody answers on.
            var silent = new UdpClient(new IPEndPoint(IPAddress.Loopback, 0));
            try
            {
                var resolver = new DnsSrvResolver(new[] { (IPEndPoint)silent.Client.LocalEndPoint }, 300, 1);
                Assert.Throws<DnsLookupException>(() => resolver.Resolve("_mpc1._tcp.example.com"), "timeout");
            }
            finally
            {
                silent.Dispose();
            }
        }

        [Test]
        public static void ParsesServerLists()
        {
            IList<IPEndPoint> list = DnsSrvResolver.ParseServerList("1.1.1.1, 8.8.8.8:5353; [2001:db8::1]:54 ::1");
            Assert.Sequence(new[] { "1.1.1.1:53", "8.8.8.8:5353", "[2001:db8::1]:54", "[::1]:53" }, list.Select(e => e.ToString()), "endpoints");
            Assert.Throws<FormatException>(() => DnsSrvResolver.ParseServerList("not-an-ip"), "bad entry");
        }
    }

    /// <summary>A tiny authoritative DNS server on 127.0.0.1 (UDP and TCP) for tests.</summary>
    internal sealed class FakeDns : IDisposable
    {
        private readonly UdpClient _udp;
        private readonly TcpListener _tcp;
        private readonly Thread _udpThread;
        private readonly Thread _tcpThread;
        private volatile bool _stopping;

        public readonly Dictionary<string, SrvRecord[]> Zone = new Dictionary<string, SrvRecord[]>(StringComparer.OrdinalIgnoreCase);
        public bool TruncateUdp { get; set; }
        public int Rcode { get; set; }
        public int TcpQueries;

        public FakeDns()
        {
            _udp = new UdpClient(new IPEndPoint(IPAddress.Loopback, 0));
            int port = ((IPEndPoint)_udp.Client.LocalEndPoint).Port;
            _tcp = new TcpListener(IPAddress.Loopback, port);
            _tcp.Start();
            EndPoint = new IPEndPoint(IPAddress.Loopback, port);

            _udpThread = new Thread(ServeUdp) { IsBackground = true };
            _tcpThread = new Thread(ServeTcp) { IsBackground = true };
            _udpThread.Start();
            _tcpThread.Start();
        }

        public IPEndPoint EndPoint { get; private set; }

        private byte[] Handle(byte[] query, bool truncate)
        {
            int pos = 12;
            string name = DnsMessage.ReadName(query, ref pos);
            if (Rcode != 0) return Answer(query, Rcode, false, new SrvRecord[0]);
            SrvRecord[] records;
            if (!Zone.TryGetValue(name, out records)) return Answer(query, DnsMessage.RcodeNameError, false, new SrvRecord[0]);
            return Answer(query, 0, truncate, truncate ? new SrvRecord[0] : records);
        }

        private void ServeUdp()
        {
            while (!_stopping)
            {
                try
                {
                    IPEndPoint remote = null;
                    byte[] query = _udp.Receive(ref remote);
                    byte[] answer = Handle(query, TruncateUdp);
                    _udp.Send(answer, answer.Length, remote);
                }
                catch (Exception)
                {
                    if (_stopping) return;
                }
            }
        }

        private void ServeTcp()
        {
            while (!_stopping)
            {
                try
                {
                    using (TcpClient client = _tcp.AcceptTcpClient())
                    using (NetworkStream s = client.GetStream())
                    {
                        Interlocked.Increment(ref TcpQueries);
                        int len = (s.ReadByte() << 8) | s.ReadByte();
                        var query = new byte[len];
                        int read = 0;
                        while (read < len) read += s.Read(query, read, len - read);
                        byte[] answer = Handle(query, false);
                        s.WriteByte((byte)(answer.Length >> 8));
                        s.WriteByte((byte)(answer.Length & 0xFF));
                        s.Write(answer, 0, answer.Length);
                    }
                }
                catch (Exception)
                {
                    if (_stopping) return;
                }
            }
        }

        /// <summary>
        /// Builds a response: the question is echoed, answer names point back at it (0xC00C), and
        /// any target equal to the first answer's target is written as a compression pointer.
        /// </summary>
        public static byte[] Answer(byte[] query, int rcode, bool truncated, IList<SrvRecord> records)
        {
            int qEnd = 12;
            DnsMessage.ReadName(query, ref qEnd);
            qEnd += 4;

            var b = new List<byte>();
            b.Add(query[0]);
            b.Add(query[1]);
            int flags = 0x8000 | 0x0100 | 0x0080 | (truncated ? 0x0200 : 0) | (rcode & 0xF);
            b.Add((byte)(flags >> 8));
            b.Add((byte)flags);
            b.AddRange(new byte[] { 0, 1, 0, (byte)records.Count, 0, 0, 0, 0 });
            b.AddRange(query.Skip(12).Take(qEnd - 12));

            int firstTargetOffset = -1;
            string firstTarget = null;
            foreach (SrvRecord r in records)
            {
                b.AddRange(new byte[] { 0xC0, 0x0C, 0, 33, 0, 1, 0, 0, 0x0E, 0x10 });
                var rdata = new List<byte>
                {
                    (byte)(r.Priority >> 8), (byte)r.Priority,
                    (byte)(r.Weight >> 8), (byte)r.Weight,
                    (byte)(r.Port >> 8), (byte)r.Port,
                };

                int targetOffset = b.Count + 2 + rdata.Count;
                if (firstTarget != null && r.Target == firstTarget)
                {
                    rdata.Add((byte)(0xC0 | (firstTargetOffset >> 8)));
                    rdata.Add((byte)firstTargetOffset);
                }
                else
                {
                    foreach (string label in r.Target.Split(new[] { '.' }, StringSplitOptions.RemoveEmptyEntries))
                    {
                        rdata.Add((byte)label.Length);
                        rdata.AddRange(Encoding.ASCII.GetBytes(label));
                    }
                    rdata.Add(0);
                    if (firstTarget == null)
                    {
                        firstTarget = r.Target;
                        firstTargetOffset = targetOffset;
                    }
                }

                b.Add((byte)(rdata.Count >> 8));
                b.Add((byte)rdata.Count);
                b.AddRange(rdata);
            }

            return b.ToArray();
        }

        public void Dispose()
        {
            _stopping = true;
            _udp.Dispose();
            _tcp.Stop();
        }
    }
}
