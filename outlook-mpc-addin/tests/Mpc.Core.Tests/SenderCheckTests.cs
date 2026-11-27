using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace Mpc.Core.Tests
{
    /// <summary>
    /// Stage 1 (sender checks). The Authentication-Results lines below are the ones OpenDKIM and
    /// OpenDMARC (Ubuntu 24.04, as in the lab) added to a chunk for green.mpc, configured as
    /// lab/aws-opentofu/templates/bootstrap.sh.tftpl does.
    /// </summary>
    public static class SenderCheckTests
    {
        private const string RelayA = "relay.provider-a.test";
        private const string RelayB = "relay.provider-b.test";

        private static string[] Passed(string relay)
        {
            return new[]
            {
                relay + "; dmarc=pass (p=reject dis=none) header.from=green.mpc",
                relay + ";\r\n\tdkim=pass (2048-bit key; unprotected) header.d=green.mpc header.i=@green.mpc header.a=rsa-sha256 header.s=mpc header.b=zhixlIZt;\r\n\tdkim-atps=neutral",
                relay + "; spf=pass smtp.mailfrom=green.mpc",
            };
        }

        /// <summary>A forged chunk: unsigned, sent from an address not in green.mpc's SPF record.</summary>
        private static string[] Forged(string relay)
        {
            return new[]
            {
                relay + "; dmarc=fail (p=reject dis=none) header.from=green.mpc",
                relay + "; dkim=none;\r\n\tdkim-atps=neutral",
                relay + "; spf=fail smtp.mailfrom=green.mpc",
            };
        }

        private static string Headers(int index, string via, string from, IEnumerable<string> authenticationResults)
        {
            // Like delivered mail: the envelope sender is the From: address (alice when there is none).
            return Headers(index, via, from, authenticationResults, "<" + (EmailAddress.GetAddress(from) ?? "alice@green.mpc") + ">");
        }

        private static string Headers(int index, string via, string from, IEnumerable<string> authenticationResults, string returnPath)
        {
            var sb = new StringBuilder();
            if (returnPath != null) sb.Append("Return-Path: ").Append(returnPath).Append("\r\n");
            sb.Append("Received: from relay.provider-c.test by mail.blue.mpc\r\n");
            foreach (string ar in authenticationResults) sb.Append("Authentication-Results: ").Append(ar).Append("\r\n");
            sb.Append("From: ").Append(from).Append("\r\n");
            sb.Append("To: Bob <bob@blue.mpc>\r\n");
            sb.Append("Subject: Project plan\r\n");
            sb.Append("X-MPC-Via: ").Append(via).Append("\r\n");
            sb.Append("X-MPC-ID: 3F2504E0-4F89-11D3-9A0C-0305E82C3301\r\n");
            sb.Append("X-MPC-Chunk: [00").Append(index).Append("/002]\r\n");
            sb.Append("X-MPC-Result:  \r\n");
            sb.Append("X-MPC-Scheme: additive-p61\r\n\r\n");
            return sb.ToString();
        }

        private static SenderCheckResult Check(string from1, IEnumerable<string> results1, string from2, IEnumerable<string> results2)
        {
            var chunks = new[]
            {
                MpcCoalescer.TryReadChunk("m1", Headers(1, RelayA, from1, results1)),
                MpcCoalescer.TryReadChunk("m2", Headers(2, RelayB, from2, results2)),
            };
            IReadOnlyList<ChunkGroup<string>> groups = MpcCoalescer.Group(chunks);
            Assert.Equal(1, groups.Count, "one message");
            Assert.True(groups[0].IsComplete, "complete");
            return SenderChecks.Evaluate(groups[0]);
        }

        private const string Alice = "Alice <alice@green.mpc>";

        [Test]
        public static void ParsesTheLabsHeaders()
        {
            AuthenticationResults dmarc, dkim, spf;
            Assert.True(AuthenticationResults.TryParse(Passed(RelayA)[0], out dmarc), "dmarc");
            Assert.True(AuthenticationResults.TryParse(Passed(RelayA)[1].Replace("\r\n", ""), out dkim), "dkim");
            Assert.True(AuthenticationResults.TryParse(RelayA + "; spf=pass (sender SPF authorized) smtp.mailfrom=green.mpc (client-ip=10.20.1.10; helo=mail.green.mpc)", out spf), "spf");

            Assert.Equal(RelayA, dmarc.AuthservId);
            Assert.Equal("dmarc", dmarc.Results[0].Method);
            Assert.Equal("pass", dmarc.Results[0].Result);
            Assert.Equal("green.mpc", dmarc.Results[0].GetProperty("header.from"));

            Assert.Sequence(new[] { "dkim=pass", "dkim-atps=neutral" }, dkim.Results.Select(r => r.ToString()), "dkim results");
            Assert.Equal("green.mpc", dkim.Results[0].GetProperty("header.d"));
            Assert.Equal("mpc", dkim.Results[0].GetProperty("header.s"));

            // The comment holds semicolons; they must not split the result.
            Assert.Equal(1, spf.Results.Count, "one SPF result");
            Assert.Equal("pass", spf.Results[0].Result);
            Assert.Equal("green.mpc", spf.Results[0].GetProperty("smtp.mailfrom"));
        }

        [Test]
        public static void ParsesOtherForms()
        {
            AuthenticationResults ar;
            // The SPF checker's line when it skips a relay: no method at all.
            Assert.True(AuthenticationResults.TryParse(RelayA + "; none (SPF check N/A for local connections - client-ip=10.20.5.10; helo=relay.provider-c.test)", out ar), "none");
            Assert.Equal(0, ar.Results.Count, "no results");

            // Version number, method version, quoted value with a semicolon, nested comment.
            Assert.True(AuthenticationResults.TryParse("mx.example.com 1; dkim/1=pass (good (very)) header.d=\"a;b.example\" reason=\"ok\"; dmarc=FAIL", out ar), "mixed");
            Assert.Equal("mx.example.com", ar.AuthservId);
            Assert.Sequence(new[] { "dkim=pass", "dmarc=fail" }, ar.Results.Select(r => r.ToString()), "results");
            Assert.Equal("a;b.example", ar.Results[0].GetProperty("header.d"));
            Assert.Equal("ok", ar.Results[0].GetProperty("reason"));

            Assert.False(AuthenticationResults.TryParse("   ", out ar), "empty");
        }

        [Test]
        public static void ReadsFromAndResultsWithTheChunk()
        {
            ReceivedChunk<string> chunk = MpcCoalescer.TryReadChunk("m", Headers(1, RelayA, Alice, Passed(RelayA)));
            Assert.Equal(Alice, chunk.From);
            Assert.Equal(3, chunk.AuthenticationResults.Count, "three headers");
            Assert.True(chunk.AuthenticationResults.All(a => a.AuthservId == RelayA), "all from relay-a");
        }

        [Test]
        public static void PassesWhenEveryChunkPassedAtItsRelay()
        {
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, Passed(RelayB));
            Assert.True(result.Passed, "passed: " + string.Join("; ", result.Problems));
        }

        [Test]
        public static void FailsAForgedChunk()
        {
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, Forged(RelayB));
            Assert.False(result.Passed, "forged chunk 2");
            Assert.True(result.Problems.Any(p => p.StartsWith("chunk 2: DMARC fail", StringComparison.Ordinal)), "DMARC: " + string.Join("; ", result.Problems));
            Assert.True(result.Problems.Any(p => p.StartsWith("chunk 2: no valid DKIM signature from green.mpc", StringComparison.Ordinal)), "DKIM");
            Assert.False(result.Problems.Any(p => p.StartsWith("chunk 1", StringComparison.Ordinal)), "chunk 1 is fine");
        }

        [Test]
        public static void TrustsOnlyTheRelayInXMpcVia()
        {
            // Chunk 2 went via relay-b, but carries only results claiming to be from relay-a
            // (for example written by the sender): they don't count.
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, Passed(RelayA));
            Assert.False(result.Passed, "wrong relay");
            Assert.Equal("chunk 2: no sender-check results from " + RelayB, result.Problems.Single());

            // Results from relay-b in another letter case, or with a trailing dot, do count.
            Assert.True(Check(Alice, Passed(RelayA), Alice, Passed("Relay.Provider-B.test.")).Passed, "case and trailing dot");
        }

        [Test]
        public static void AForgedPassDoesNotOutvoteTheRelaysFail()
        {
            // What Sriram found: a sender adds "pass" lines in the relay's name. If any survive
            // next to the relay's real results, the real "fail" and "none" still count.
            var forgedPlusReal = Forged(RelayB).Concat(Passed(RelayB)).ToArray();
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, forgedPlusReal);
            Assert.False(result.Passed, "fake passes beside real failures");
            Assert.Sequence(new[]
            {
                "chunk 2: SPF results disagree (pass and fail) at " + RelayB,
                "chunk 2: DKIM results disagree (pass and none) at " + RelayB,
                "chunk 2: DMARC results disagree (pass and fail) at " + RelayB,
            }, result.Problems, "problems");
        }

        [Test]
        public static void FailsAReplayedChunk()
        {
            // A genuine signed chunk sent again from another server: DKIM and DMARC still pass,
            // but SPF doesn't.
            var replayed = new[] { Passed(RelayB)[0], Passed(RelayB)[1], RelayB + "; spf=fail smtp.mailfrom=green.mpc" };
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, replayed);
            Assert.Equal("chunk 2: SPF fail (at " + RelayB + ")", result.Problems.Single());
        }

        [Test]
        public static void EachCheckMustBeThere()
        {
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, new[] { Passed(RelayB)[1] });
            Assert.Sequence(new[]
            {
                "chunk 2: SPF was not checked (at " + RelayB + ")",
                "chunk 2: DMARC was not checked for green.mpc (at " + RelayB + ")",
            }, result.Problems, "only DKIM");
        }

        [Test]
        public static void FailsWithoutResults()
        {
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, new string[0]);
            Assert.Equal("chunk 2: no sender-check results from " + RelayB, result.Problems.Single());
        }

        [Test]
        public static void SpfAloneIsNotEnough()
        {
            // DMARC passes on SPF, but the chunk isn't signed: it may have been changed.
            var spfOnly = new[] { RelayB + "; dmarc=pass header.from=green.mpc", RelayB + "; dkim=none", RelayB + "; spf=pass smtp.mailfrom=green.mpc" };
            SenderCheckResult result = Check(Alice, Passed(RelayA), Alice, spfOnly);
            Assert.Equal("chunk 2: no valid DKIM signature from green.mpc (at " + RelayB + ")", result.Problems.Single());
        }

        [Test]
        public static void DkimMustBeFromTheFromDomain()
        {
            var otherSigner = new[] { RelayB + "; dmarc=pass header.from=green.mpc", RelayB + "; dkim=pass header.d=evil.example", RelayB + "; spf=pass smtp.mailfrom=green.mpc" };
            Assert.False(Check(Alice, Passed(RelayA), Alice, otherSigner).Passed, "signed by another domain");

            var dmarcForAnotherDomain = new[] { RelayB + "; dmarc=pass header.from=evil.example", RelayB + "; dkim=pass header.d=green.mpc" };
            Assert.False(Check(Alice, Passed(RelayA), Alice, dmarcForAnotherDomain).Passed, "DMARC for another From domain");
        }

        [Test]
        public static void ChunksMustHaveTheSameSender()
        {
            SenderCheckResult result = Check(Alice, Passed(RelayA), "Mallory <mallory@green.mpc>", Passed(RelayB));
            Assert.Equal("the chunks have different From: addresses (alice@green.mpc, mallory@green.mpc)", result.Problems.Single());

            // The display name and letter case don't matter.
            Assert.True(Check(Alice, Passed(RelayA), "ALICE@Green.MPC", Passed(RelayB)).Passed, "same address");
        }

        [Test]
        public static void FailsWithoutAFromAddress()
        {
            SenderCheckResult result = Check(Alice, Passed(RelayA), "undisclosed-recipients:;", Passed(RelayB));
            Assert.Equal("chunk 2: no From: address", result.Problems.Single());
        }

        private static SenderCheckSelection<string> Select(params ReceivedChunk<string>[] chunks)
        {
            IReadOnlyList<ChunkGroup<string>> groups = MpcCoalescer.Group(chunks);
            Assert.Equal(1, groups.Count, "one message");
            return SenderChecks.SelectPassing(groups[0]);
        }

        private static ReceivedChunk<string> Chunk(string item, int index, string via, string from, IEnumerable<string> results)
        {
            return MpcCoalescer.TryReadChunk(item, Headers(index, via, from, results));
        }

        private static string[] Replayed(string relay)
        {
            // A genuine signed chunk sent again from another server: only SPF fails.
            return new[] { Passed(relay)[0], Passed(relay)[1], relay + "; spf=fail smtp.mailfrom=green.mpc" };
        }

        [Test]
        public static void AReplayedDuplicateIsKeptAndReported()
        {
            SenderCheckSelection<string> selection = Select(
                Chunk("genuine1", 1, RelayA, Alice, Passed(RelayA)),
                Chunk("genuine2", 2, RelayB, Alice, Passed(RelayB)),
                Chunk("replay1", 1, RelayA, Alice, Replayed(RelayA)));

            Assert.Sequence(new[] { "genuine1", "genuine2" }, selection.Group.Chunks.Select(c => c.Item), "chunks used");
            Assert.True(SenderChecks.Evaluate(selection.Group).Passed, "the message passes");
            Assert.Equal(0, selection.Group.Extras.Count, "no harmless extras");
            Assert.Sequence(new[] { "replay1" }, selection.Suspicious.Select(c => c.Item), "kept");
            Assert.Equal("duplicate of chunk 1: SPF fail (at " + RelayA + ")", selection.SuspiciousDetails.Single());
        }

        [Test]
        public static void APassingCopyIsUsedEvenWhenAForgedOneComesFirst()
        {
            // MpcCoalescer.Group picks the first copy it sees; a forged copy listed first must not
            // stop the genuine message.
            SenderCheckSelection<string> selection = Select(
                Chunk("forged1", 1, RelayA, Alice, Forged(RelayA)),
                Chunk("genuine1", 1, RelayA, Alice, Passed(RelayA)),
                Chunk("genuine2", 2, RelayB, Alice, Passed(RelayB)));

            Assert.Sequence(new[] { "genuine1", "genuine2" }, selection.Group.Chunks.Select(c => c.Item), "chunks used");
            Assert.True(SenderChecks.Evaluate(selection.Group).Passed, "the message passes");
            Assert.Sequence(new[] { "forged1" }, selection.Suspicious.Select(c => c.Item), "kept");
            Assert.True(selection.SuspiciousDetails.Single().StartsWith("duplicate of chunk 1: SPF fail", StringComparison.Ordinal), selection.SuspiciousDetails.Single());
        }

        [Test]
        public static void AnIdenticalPassingDuplicateIsAnExtra()
        {
            SenderCheckSelection<string> selection = Select(
                Chunk("genuine1", 1, RelayA, Alice, Passed(RelayA)),
                Chunk("copy1", 1, RelayA, Alice, Passed(RelayA)),
                Chunk("genuine2", 2, RelayB, Alice, Passed(RelayB)));

            Assert.Sequence(new[] { "genuine1", "genuine2" }, selection.Group.Chunks.Select(c => c.Item), "chunks used");
            Assert.Sequence(new[] { "copy1" }, selection.Group.Extras.Select(c => c.Item), "extra, deleted as before");
            Assert.Equal(0, selection.Suspicious.Count, "nothing suspicious");
        }

        [Test]
        public static void ADuplicateFromSomeoneElseIsSuspicious()
        {
            SenderCheckSelection<string> selection = Select(
                Chunk("genuine1", 1, RelayA, Alice, Passed(RelayA)),
                Chunk("genuine2", 2, RelayB, Alice, Passed(RelayB)),
                Chunk("other2", 2, RelayB, "Mallory <mallory@green.mpc>", Passed(RelayB)));

            Assert.Equal("duplicate of chunk 2: From: mallory@green.mpc differs from the other chunks", selection.SuspiciousDetails.Single());
            Assert.Equal(0, selection.Group.Extras.Count, "not an extra");
        }

        [Test]
        public static void WhenEveryCopyFailsTheMessageFails()
        {
            SenderCheckSelection<string> selection = Select(
                Chunk("forged1", 1, RelayA, Alice, Forged(RelayA)),
                Chunk("replay1", 1, RelayA, Alice, Replayed(RelayA)),
                Chunk("genuine2", 2, RelayB, Alice, Passed(RelayB)));

            Assert.Equal("forged1", selection.Group.Chunks[0].Item);
            Assert.False(SenderChecks.Evaluate(selection.Group).Passed, "the message fails");
            Assert.Sequence(new[] { "replay1" }, selection.Suspicious.Select(c => c.Item), "the other copy");
        }

        private static SenderCheckResult CheckWithReturnPath(string from, string returnPath)
        {
            var chunks = new[]
            {
                MpcCoalescer.TryReadChunk("m1", Headers(1, RelayA, from, Passed(RelayA), returnPath)),
                MpcCoalescer.TryReadChunk("m2", Headers(2, RelayB, from, Passed(RelayB), returnPath)),
            };
            return SenderChecks.Evaluate(MpcCoalescer.Group(chunks).Single());
        }

        [Test]
        public static void ReadsTheReturnPath()
        {
            ReceivedChunk<string> chunk = MpcCoalescer.TryReadChunk("m", Headers(1, RelayA, Alice, Passed(RelayA), "<alice@green.mpc>"));
            Assert.Equal("<alice@green.mpc>", chunk.ReturnPath);
        }

        [Test]
        public static void FromMustBeTheEnvelopeSender()
        {
            // Gap E: alice logs in as herself (so the envelope sender is hers and SPF passes) but
            // writes another green.mpc address in From:; her server still signs it, so SPF, DKIM
            // and DMARC all pass at the relay.
            SenderCheckResult result = CheckWithReturnPath("CEO <ceo@green.mpc>", "<alice@green.mpc>");
            Assert.False(result.Passed, "a From: that isn't the envelope sender");
            Assert.Sequence(new[]
            {
                "chunk 1: From: ceo@green.mpc is not the envelope sender alice@green.mpc",
                "chunk 2: From: ceo@green.mpc is not the envelope sender alice@green.mpc",
            }, result.Problems, "problems");

            Assert.True(CheckWithReturnPath(Alice, "<ALICE@green.mpc>").Passed, "letter case doesn't matter");
        }

        [Test]
        public static void FailsWithoutAReturnPath()
        {
            SenderCheckResult result = CheckWithReturnPath(Alice, null);
            Assert.Sequence(new[]
            {
                "chunk 1: no envelope sender (Return-Path)",
                "chunk 2: no envelope sender (Return-Path)",
            }, result.Problems, "problems");

            Assert.False(CheckWithReturnPath(Alice, "<>").Passed, "an empty envelope sender (a bounce)");
        }

        [Test]
        public static void GetsTheAddress()
        {
            Assert.Equal("alice@green.mpc", EmailAddress.GetAddress("Alice <Alice@Green.MPC>"));
            Assert.Equal("alice@green.mpc", EmailAddress.GetAddress(" alice@green.mpc "));
            Assert.Null(EmailAddress.GetAddress("Alice"), "no @");
            Assert.Null(EmailAddress.GetAddress("@green.mpc"), "no local part");
            Assert.Null(EmailAddress.GetAddress(null), "null");
        }

        [Test]
        public static void RequireSenderChecksSetting()
        {
            Assert.True(new MpcSettings().RequireSenderChecks, "on by default");

            string path = Path.Combine(Path.GetTempPath(), "mpc-settings-" + Guid.NewGuid().ToString("N") + ".ini");
            try
            {
                File.WriteAllText(path, "RequireSenderChecks=false\n");
                Assert.False(MpcSettings.Load(path).RequireSenderChecks, "turned off");

                var settings = new MpcSettings();
                settings.Save(path);
                Assert.True(File.ReadAllText(path).Contains("RequireSenderChecks=true"), "saved");
                Assert.True(MpcSettings.Load(path).RequireSenderChecks, "read back");
            }
            finally
            {
                File.Delete(path);
            }
        }
    }
}
