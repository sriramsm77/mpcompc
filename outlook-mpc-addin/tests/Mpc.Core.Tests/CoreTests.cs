using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Mpc.Core.Tests
{
    public static class ChunkValueTests
    {
        [Test]
        public static void FormatsLikeSprintf()
        {
            Assert.Equal("[001/002]", new ChunkValue(1, 2).ToString());
            Assert.Equal("[000/000]", ChunkValue.Coalesced.ToString());
            Assert.Equal("[012/120]", new ChunkValue(12, 120).ToString());
        }

        [Test]
        public static void ParsesHeaderValues()
        {
            ChunkValue v;
            Assert.True(ChunkValue.TryParse("[002/002]", out v), "parse");
            Assert.Equal(new ChunkValue(2, 2), v);
            Assert.True(ChunkValue.TryParse("  [1/3] ", out v), "parse with spaces");
            Assert.Equal(new ChunkValue(1, 3), v);
            Assert.True(ChunkValue.TryParse("[000/000]", out v) && v.IsCoalesced, "coalesced");
            Assert.False(ChunkValue.TryParse("1/2", out v), "missing brackets");
            Assert.False(ChunkValue.TryParse("", out v), "empty");
            Assert.False(ChunkValue.TryParse(null, out v), "null");
            Assert.False(new ChunkValue(3, 2).IsValidChunk, "index > total");
            Assert.False(ChunkValue.Coalesced.IsValidChunk, "coalesced is not a chunk");
        }
    }

    public static class EmailAddressTests
    {
        [Test]
        public static void ExtractsDomain()
        {
            Assert.Equal("example.com", EmailAddress.GetDomain("user@example.com"));
            Assert.Equal("example.com", EmailAddress.GetDomain("Some One <User@Example.COM>"));
            Assert.Equal("example.com", EmailAddress.GetDomain("\"a@b\" <user@example.com>"));
            Assert.Null(EmailAddress.GetDomain("no-at-sign"), "no @");
            Assert.Null(EmailAddress.GetDomain("user@"), "nothing after @");
            Assert.Null(EmailAddress.GetDomain(null), "null");
        }
    }

    public static class BodyChunkerTests
    {
        /// <summary>Chunk sizes chunkifyEMailBody() in send.c produces for an ASCII body.</summary>
        private static IEnumerable<int> CSizes(int size, int n)
        {
            int chunk = size / n;
            for (int i = 0; i < n - 1; i++) yield return chunk;
            yield return size - chunk * (n - 1);
        }

        [Test]
        public static void MatchesCChunkSizesForAscii()
        {
            for (int n = 1; n <= 3; n++)
            {
                for (int size = 1; size <= 64; size++)
                {
                    string body = new string(Enumerable.Range(0, size).Select(i => (char)('a' + i % 26)).ToArray());
                    IReadOnlyList<string> parts = BodyChunker.Split(body, n);
                    Assert.Sequence(CSizes(size, n), parts.Select(p => p.Length), "size " + size + " into " + n);
                    Assert.Equal(body, string.Concat(parts), "rejoined");
                }
            }
        }

        [Test]
        public static void NeverSplitsAMultiByteCharacter()
        {
            string body = "aé€😀b日本語\r\nmore text ü";
            for (int n = 1; n <= 5; n++)
            {
                IReadOnlyList<string> parts = BodyChunker.Split(body, n);
                Assert.Equal(n, parts.Count, "chunk count");
                Assert.Equal(body, string.Concat(parts), "rejoined with " + n);
                foreach (string p in parts) Assert.False(p.Contains('�'), "no replacement characters");
            }
        }

        [Test]
        public static void AlwaysReturnsRequestedCount()
        {
            Assert.Sequence(new[] { "", "" }, BodyChunker.Split("", 2), "empty body");
            Assert.Sequence(new[] { "", "x" }, BodyChunker.Split("x", 2), "1 byte");
            Assert.Throws<ArgumentOutOfRangeException>(() => BodyChunker.Split("x", 0), "zero chunks");
        }
    }

    public static class SendPlannerTests
    {
        [Test]
        public static void SetsTheSameHeadersAsSendC()
        {
            var servers = new[] { new SrvRecord(0, 0, 25, "relay1.example.net"), new SrvRecord(0, 0, 25, "relay2.example.org.") };
            string id = MpcSendPlanner.NewMpcId();
            IReadOnlyList<OutgoingChunk> chunks = MpcSendPlanner.Plan("Hello, multi-party world!", servers, id);

            Assert.Equal(2, chunks.Count, "chunks");
            Assert.Sequence(
                new[] { "X-MPC-Via", "X-MPC-ID", "X-MPC-Chunk", "X-MPC-Result" },
                chunks[0].Headers.Select(h => h.Key), "header names");
            Assert.Equal("relay1.example.net", chunks[0].Headers[0].Value);
            Assert.Equal("relay2.example.org", chunks[1].Via, "trailing dot removed");
            Assert.Equal(id, chunks[1].Headers[1].Value);
            Assert.Equal("[001/002]", chunks[0].Headers[2].Value);
            Assert.Equal("[002/002]", chunks[1].Headers[2].Value);
            Assert.Equal(" ", chunks[0].Headers[3].Value);
            Assert.False(chunks[0].CarriesAttachments, "attachments not on first");
            Assert.True(chunks[1].CarriesAttachments, "attachments on last");
            Assert.Equal("Hello, multi-party world!", chunks[0].Body + chunks[1].Body);
        }

        [Test]
        public static void IdIsUpperCaseUuid()
        {
            string id = MpcSendPlanner.NewMpcId();
            Assert.Equal(36, id.Length, "length");
            Assert.Equal(id.ToUpperInvariant(), id, "upper case");
            Guid g;
            Assert.True(Guid.TryParse(id, out g), "is a UUID");
        }
    }

    public static class InternetHeadersTests
    {
        private const string Raw =
            "Received: from relay1.example.net\r\n\tby mx.example.com; Mon, 14 Oct 2024 04:31:37 +0000\r\n" +
            "x-mpc-id: 3F2504E0-4F89-11D3-9A0C-0305E82C3301\r\n" +
            "X-MPC-Chunk: [002/002]\r\n" +
            "X-MPC-Via:\r\n relay2.example.org\r\n" +
            "Subject: hi\r\n" +
            "\r\n";

        [Test]
        public static void ReadsFoldedAndCaseInsensitiveFields()
        {
            Assert.Equal("3F2504E0-4F89-11D3-9A0C-0305E82C3301", InternetHeaders.GetFirst(Raw, "X-MPC-ID"));
            Assert.Equal("relay2.example.org", InternetHeaders.GetFirst(Raw, "x-mpc-via"));
            Assert.Equal("from relay1.example.net by mx.example.com; Mon, 14 Oct 2024 04:31:37 +0000", InternetHeaders.GetFirst(Raw, "Received"));
            Assert.Null(InternetHeaders.GetFirst(Raw, "X-MPC-Result"), "missing field");
            Assert.Null(InternetHeaders.GetFirst("Subject: a\n\nX-MPC-ID: in-body", "X-MPC-ID"), "stops at blank line");
        }

        [Test]
        public static void ReplacesAndAddsFields()
        {
            string updated = InternetHeaders.SetValue(Raw, "X-MPC-Via", "relay1.example.net, relay2.example.org");
            Assert.Equal("relay1.example.net, relay2.example.org", InternetHeaders.GetFirst(updated, "X-MPC-Via"));
            Assert.Equal("hi", InternetHeaders.GetFirst(updated, "Subject"), "following field intact");
            Assert.True(updated.EndsWith("\r\n\r\n", StringComparison.Ordinal), "ending preserved");
            Assert.False(updated.Contains("\n relay2"), "old folded line removed");

            string added = InternetHeaders.SetValue("Subject: a\r\n", "X-MPC-Chunk", "[000/000]");
            Assert.Equal("Subject: a\r\nX-MPC-Chunk: [000/000]\r\n", added);
        }
    }

    public static class CoalescerTests
    {
        private static string Headers(string id, string chunk, string via)
        {
            var sb = new StringBuilder("From: sender@example.org\r\n");
            if (id != null) sb.Append("X-MPC-ID: ").Append(id).Append("\r\n");
            if (chunk != null) sb.Append("X-MPC-Chunk: ").Append(chunk).Append("\r\n");
            if (via != null) sb.Append("X-MPC-Via: ").Append(via).Append("\r\n");
            return sb.Append("\r\n").ToString();
        }

        [Test]
        public static void SkipsNonChunks()
        {
            Assert.Null(MpcCoalescer.TryReadChunk("m", Headers(null, "[001/002]", "a")), "no ID");
            Assert.Null(MpcCoalescer.TryReadChunk("m", Headers("ID", "[000/000]", "a, b")), "already rejoined");
            Assert.Null(MpcCoalescer.TryReadChunk("m", Headers("ID", "junk", "a")), "bad chunk value");
            Assert.Null(MpcCoalescer.TryReadChunk("m", Headers("ID", "[001/002]", null)), "no Via");
            Assert.NotNull(MpcCoalescer.TryReadChunk("m", Headers("ID", "[001/002]", "a")), "valid chunk");
        }

        [Test]
        public static void OrdersChunksThatArriveOutOfOrder()
        {
            // mail.c's insertion loop dereferences a NULL "prev" in this case; the port sorts.
            var chunks = new[]
            {
                MpcCoalescer.TryReadChunk("second", Headers("A", "[002/002]", "relay2")),
                MpcCoalescer.TryReadChunk("first", Headers("A", "[001/002]", "relay1")),
            };
            ChunkGroup<string> g = MpcCoalescer.Group(chunks).Single();
            Assert.True(g.IsComplete, "complete");
            Assert.Sequence(new[] { "first", "second" }, g.Chunks.Select(c => c.Item), "order");
            Assert.Equal("relay1, relay2", g.MergedVia);
        }

        [Test]
        public static void WaitsForMissingChunks()
        {
            var chunks = new[]
            {
                MpcCoalescer.TryReadChunk("x", Headers("A", "[001/003]", "r1")),
                MpcCoalescer.TryReadChunk("z", Headers("A", "[003/003]", "r3")),
            };
            ChunkGroup<string> g = MpcCoalescer.Group(chunks).Single();
            Assert.False(g.IsComplete, "incomplete");
            Assert.Sequence(new[] { 2 }, g.MissingChunkNumbers, "missing");
        }

        [Test]
        public static void SeparatesGroupsAndDuplicates()
        {
            var chunks = new[]
            {
                MpcCoalescer.TryReadChunk("a1", Headers("A", "[001/002]", "r1")),
                MpcCoalescer.TryReadChunk("b1", Headers("b", "[001/001]", "r9")),
                MpcCoalescer.TryReadChunk("a1-again", Headers("a", "[001/002]", "r1")),
                MpcCoalescer.TryReadChunk("a2", Headers("A", "[002/002]", "r2")),
            };
            IReadOnlyList<ChunkGroup<string>> groups = MpcCoalescer.Group(chunks);
            Assert.Equal(2, groups.Count, "groups");

            ChunkGroup<string> a = groups.Single(g => g.MpcId == "A");
            Assert.True(a.IsComplete, "A complete");
            Assert.Sequence(new[] { "a1", "a2" }, a.Chunks.Select(c => c.Item), "A chunks");
            Assert.Sequence(new[] { "a1-again" }, a.Extras.Select(c => c.Item), "A extras");

            ChunkGroup<string> b = groups.Single(g => g.MpcId == "b");
            Assert.True(b.IsComplete, "single-chunk message is complete");
        }

        [Test]
        public static void RoundTripsThroughSendAndReceive()
        {
            string body = "Line one\r\nLine two with ünïcödé and 😀\r\n";
            var servers = new[] { new SrvRecord(0, 0, 25, "relay1.example.net"), new SrvRecord(0, 0, 25, "relay2.example.org") };
            IReadOnlyList<OutgoingChunk> sent = MpcSendPlanner.Plan(body, servers, MpcSendPlanner.NewMpcId());

            // Deliver in reverse order, rendering each chunk's headers as a receiving server would.
            var received = sent.Reverse().Select(c =>
            {
                string raw = string.Concat(c.Headers.Select(h => h.Key + ": " + h.Value + "\r\n")) + "\r\n";
                return MpcCoalescer.TryReadChunk(c.Body, raw);
            }).ToList();

            ChunkGroup<string> g = MpcCoalescer.Group(received).Single();
            Assert.True(g.IsComplete, "complete");
            Assert.Equal(body, MpcCoalescer.MergeBodies(g.Chunks.Select(c => c.Item)), "body");
            Assert.Equal("relay1.example.net, relay2.example.org", g.MergedVia, "via");
        }
    }

    public static class DiscoveryTests
    {
        private sealed class FakeResolver : ISrvResolver
        {
            public readonly Dictionary<string, SrvRecord[]> Answers = new Dictionary<string, SrvRecord[]>(StringComparer.OrdinalIgnoreCase);
            public readonly List<string> Asked = new List<string>();

            public IReadOnlyList<SrvRecord> Resolve(string name)
            {
                Asked.Add(name);
                SrvRecord[] r;
                return Answers.TryGetValue(name, out r) ? r : new SrvRecord[0];
            }
        }

        [Test]
        public static void QueriesMpcNamesAndStopsAtFirstMiss()
        {
            var resolver = new FakeResolver();
            resolver.Answers["_mpc1._tcp.example.com"] = new[] { new SrvRecord(0, 0, 25, "relay1.example.net") };
            resolver.Answers["_mpc3._tcp.example.com"] = new[] { new SrvRecord(0, 0, 25, "never-used") };

            IReadOnlyList<SrvRecord> servers = MpcServerDiscovery.DiscoverServers(resolver, "example.com", 3);
            Assert.Sequence(new[] { "relay1.example.net" }, servers.Select(s => s.Target), "servers");
            Assert.Sequence(new[] { "_mpc1._tcp.example.com", "_mpc2._tcp.example.com" }, resolver.Asked, "queries");
        }

        [Test]
        public static void UsesAtMostTwoServersByDefault()
        {
            var resolver = new FakeResolver();
            for (int i = 1; i <= 3; i++) resolver.Answers["_mpc" + i + "._tcp.example.com"] = new[] { new SrvRecord(0, 0, 25, "r" + i) };
            Assert.Equal(2, MpcServerDiscovery.DiscoverServers(resolver, "example.com").Count, "MAX_MPC_SERVERS");
        }

        [Test]
        public static void PicksLowestPriorityAndIgnoresDotTarget()
        {
            Assert.Equal("b", MpcServerDiscovery.PickRecord(new[] { new SrvRecord(20, 0, 25, "a"), new SrvRecord(10, 0, 25, "b") }).Target);
            Assert.Null(MpcServerDiscovery.PickRecord(new[] { new SrvRecord(0, 0, 0, "") }), "'.' means no service");
        }
    }

    public static class SettingsTests
    {
        [Test]
        public static void SavesAndLoads()
        {
            string path = Path.Combine(Path.GetTempPath(), "mpc-settings-" + Guid.NewGuid() + ".ini");
            try
            {
                Assert.True(MpcSettings.Load(path).AutoSplitOnSend, "default when missing");

                var s = new MpcSettings { AutoSplitOnSend = false, RejoinLookbackDays = 7, DnsServers = "1.1.1.1, 8.8.8.8" };
                s.Save(path);
                MpcSettings loaded = MpcSettings.Load(path);
                Assert.False(loaded.AutoSplitOnSend, "AutoSplitOnSend");
                Assert.True(loaded.AutoRejoinNewMail, "AutoRejoinNewMail");
                Assert.Equal(7, loaded.RejoinLookbackDays);
                Assert.Equal("1.1.1.1, 8.8.8.8", loaded.DnsServers);

                File.WriteAllText(path, "AutoSplitOnSend = maybe\nDnsTimeoutMs=abc\n# comment\nUnknown=1\n");
                loaded = MpcSettings.Load(path);
                Assert.True(loaded.AutoSplitOnSend, "bad bool keeps default");
                Assert.Equal(2000, loaded.DnsTimeoutMs, "bad int keeps default");
            }
            finally
            {
                File.Delete(path);
            }
        }
    }
}
