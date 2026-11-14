using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;

namespace Mpc.Core
{
    /// <summary>
    /// Sends SRV queries straight to DNS servers over UDP, falling back to TCP when the answer
    /// is truncated. By default it uses the DNS servers configured on this computer.
    /// </summary>
    public sealed class DnsSrvResolver : ISrvResolver
    {
        private readonly IList<IPEndPoint> _servers;
        private readonly int _timeoutMs;
        private readonly int _attemptsPerServer;
        private readonly Random _random = new Random();

        /// <param name="servers">DNS servers to ask; null means the system's DNS servers.</param>
        /// <param name="timeoutMs">Time to wait for each answer.</param>
        /// <param name="attemptsPerServer">How many times to ask each server before moving on.</param>
        public DnsSrvResolver(IEnumerable<IPEndPoint> servers = null, int timeoutMs = 2000, int attemptsPerServer = 2)
        {
            _servers = servers == null ? null : servers.ToList();
            _timeoutMs = Math.Max(100, timeoutMs);
            _attemptsPerServer = Math.Max(1, attemptsPerServer);
        }

        public IReadOnlyList<SrvRecord> Resolve(string name)
        {
            IList<IPEndPoint> servers = _servers ?? GetSystemDnsServers();
            if (servers.Count == 0)
            {
                throw new DnsLookupException("No DNS servers are configured on this computer.");
            }

            Exception lastError = null;
            foreach (IPEndPoint server in servers)
            {
                for (int attempt = 0; attempt < _attemptsPerServer; attempt++)
                {
                    try
                    {
                        ushort id = NextId();
                        byte[] query = DnsMessage.BuildQuery(id, name, DnsMessage.TypeSrv);

                        DnsResponse response = DnsMessage.ParseResponse(QueryUdp(server, query), id);
                        if (response.Truncated)
                        {
                            response = DnsMessage.ParseResponse(QueryTcp(server, query), id);
                        }

                        if (response.Rcode == DnsMessage.RcodeNoError)
                        {
                            return response.Records;
                        }
                        if (response.Rcode == DnsMessage.RcodeNameError)
                        {
                            return new SrvRecord[0];
                        }

                        lastError = new DnsLookupException(
                            "DNS server " + server + " answered with error code " + response.Rcode + " for " + name + ".");
                        break; // a server-side error will not change on retry; try the next server
                    }
                    catch (SocketException ex)
                    {
                        lastError = ex;
                    }
                    catch (IOException ex)
                    {
                        lastError = ex;
                    }
                    catch (FormatException ex)
                    {
                        lastError = ex;
                    }
                    catch (TimeoutException ex)
                    {
                        lastError = ex;
                    }
                }
            }

            throw new DnsLookupException(
                "Could not look up " + name + ": " + (lastError == null ? "no answer" : lastError.Message), lastError);
        }

        /// <summary>DNS servers configured on active network interfaces, IPv4 first.</summary>
        public static IList<IPEndPoint> GetSystemDnsServers()
        {
            var found = new List<IPAddress>();
            NetworkInterface[] interfaces;
            try
            {
                interfaces = NetworkInterface.GetAllNetworkInterfaces();
            }
            catch (NetworkInformationException)
            {
                return new List<IPEndPoint>();
            }

            foreach (NetworkInterface ni in interfaces)
            {
                if (ni.OperationalStatus != OperationalStatus.Up) continue;
                if (ni.NetworkInterfaceType == NetworkInterfaceType.Loopback) continue;

                IPInterfaceProperties props;
                try
                {
                    props = ni.GetIPProperties();
                }
                catch (NetworkInformationException)
                {
                    continue;
                }

                foreach (IPAddress address in props.DnsAddresses)
                {
                    // Windows lists deprecated fec0:: site-local resolvers when no IPv6 DNS is set.
                    if (address.AddressFamily == AddressFamily.InterNetworkV6 && address.IsIPv6SiteLocal) continue;
                    if (!found.Contains(address)) found.Add(address);
                }
            }

            return found
                .OrderBy(a => a.AddressFamily == AddressFamily.InterNetwork ? 0 : 1)
                .Select(a => new IPEndPoint(a, 53))
                .ToList();
        }

        /// <summary>Parses "1.1.1.1, 8.8.8.8:53, [2001:db8::1]:5353" into endpoints.</summary>
        public static IList<IPEndPoint> ParseServerList(string list)
        {
            var result = new List<IPEndPoint>();
            if (string.IsNullOrWhiteSpace(list)) return result;

            foreach (string raw in list.Split(new[] { ',', ';', ' ' }, StringSplitOptions.RemoveEmptyEntries))
            {
                string item = raw.Trim();
                int port = 53;
                string host = item;

                if (item.StartsWith("[", StringComparison.Ordinal))
                {
                    int close = item.IndexOf(']');
                    if (close < 0) throw new FormatException("Bad DNS server: " + item);
                    host = item.Substring(1, close - 1);
                    if (close + 1 < item.Length)
                    {
                        if (item[close + 1] != ':') throw new FormatException("Bad DNS server: " + item);
                        port = int.Parse(item.Substring(close + 2), System.Globalization.CultureInfo.InvariantCulture);
                    }
                }
                else if (item.Count(c => c == ':') == 1)
                {
                    int colon = item.IndexOf(':');
                    host = item.Substring(0, colon);
                    port = int.Parse(item.Substring(colon + 1), System.Globalization.CultureInfo.InvariantCulture);
                }

                IPAddress address;
                if (!IPAddress.TryParse(host, out address)) throw new FormatException("Bad DNS server: " + item);
                result.Add(new IPEndPoint(address, port));
            }

            return result;
        }

        private ushort NextId()
        {
            lock (_random)
            {
                return (ushort)_random.Next(0, 65536);
            }
        }

        private byte[] QueryUdp(IPEndPoint server, byte[] query)
        {
            using (var udp = new UdpClient(server.AddressFamily))
            {
                udp.Client.ReceiveTimeout = _timeoutMs;
                udp.Client.SendTimeout = _timeoutMs;
                udp.Connect(server);
                udp.Send(query, query.Length);

                IPEndPoint remote = null;
                return udp.Receive(ref remote);
            }
        }

        private byte[] QueryTcp(IPEndPoint server, byte[] query)
        {
            using (var tcp = new TcpClient(server.AddressFamily))
            {
                tcp.ReceiveTimeout = _timeoutMs;
                tcp.SendTimeout = _timeoutMs;
                if (!tcp.ConnectAsync(server.Address, server.Port).Wait(_timeoutMs))
                {
                    throw new TimeoutException("Timed out connecting to DNS server " + server + " over TCP.");
                }

                NetworkStream stream = tcp.GetStream();
                var framed = new byte[query.Length + 2];
                framed[0] = (byte)(query.Length >> 8);
                framed[1] = (byte)(query.Length & 0xFF);
                Buffer.BlockCopy(query, 0, framed, 2, query.Length);
                stream.Write(framed, 0, framed.Length);

                byte[] lengthBytes = ReadExactly(stream, 2);
                int length = (lengthBytes[0] << 8) | lengthBytes[1];
                return ReadExactly(stream, length);
            }
        }

        private static byte[] ReadExactly(Stream stream, int count)
        {
            var buffer = new byte[count];
            int read = 0;
            while (read < count)
            {
                int n = stream.Read(buffer, read, count - read);
                if (n == 0) throw new IOException("DNS server closed the TCP connection early.");
                read += n;
            }
            return buffer;
        }
    }
}
