using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace Mpc.Core
{
    /// <summary>One result in an Authentication-Results header, such as "dkim=pass header.d=green.mpc".</summary>
    public sealed class AuthResult
    {
        public AuthResult(string method, string result, IDictionary<string, string> properties)
        {
            Method = (method ?? string.Empty).ToLowerInvariant();
            Result = (result ?? string.Empty).ToLowerInvariant();
            Properties = new Dictionary<string, string>(properties ?? new Dictionary<string, string>(), StringComparer.OrdinalIgnoreCase);
        }

        /// <summary>"spf", "dkim", "dmarc", ... (lower case, without a version).</summary>
        public string Method { get; private set; }

        /// <summary>"pass", "fail", "none", ... (lower case).</summary>
        public string Result { get; private set; }

        /// <summary>Properties such as header.d, header.from or smtp.mailfrom, and reason.</summary>
        public IReadOnlyDictionary<string, string> Properties { get; private set; }

        public string GetProperty(string name)
        {
            string value;
            return Properties.TryGetValue(name, out value) ? value : null;
        }

        public override string ToString()
        {
            return Method + "=" + Result;
        }
    }

    /// <summary>
    /// A parsed Authentication-Results header (RFC 8601): the name of the server that ran the
    /// checks (authserv-id) and its results. Comments in parentheses are ignored.
    /// </summary>
    public sealed class AuthenticationResults
    {
        private AuthenticationResults(string authservId, IReadOnlyList<AuthResult> results)
        {
            AuthservId = authservId;
            Results = results;
        }

        /// <summary>The server that added the header, e.g. "relay.provider-a.test".</summary>
        public string AuthservId { get; private set; }

        public IReadOnlyList<AuthResult> Results { get; private set; }

        /// <summary>Parses one header value. Returns false when it has no authserv-id.</summary>
        public static bool TryParse(string value, out AuthenticationResults parsed)
        {
            parsed = null;
            if (string.IsNullOrWhiteSpace(value)) return false;

            List<string> parts = SplitOutsideQuotes(RemoveComments(value), ';');
            List<string> idTokens = Tokens(parts[0]);
            if (idTokens.Count == 0) return false;

            var results = new List<AuthResult>();
            for (int i = 1; i < parts.Count; i++)
            {
                List<string> tokens = Tokens(parts[i]);
                if (tokens.Count == 0) continue;

                // "method[/version]=result"; a lone "none" means no checks were run.
                int eq = tokens[0].IndexOf('=');
                if (eq <= 0) continue;
                string method = tokens[0].Substring(0, eq);
                int slash = method.IndexOf('/');
                if (slash >= 0) method = method.Substring(0, slash);
                string result = Unquote(tokens[0].Substring(eq + 1));

                var properties = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
                for (int t = 1; t < tokens.Count; t++)
                {
                    int peq = tokens[t].IndexOf('=');
                    if (peq <= 0) continue;
                    properties[tokens[t].Substring(0, peq)] = Unquote(tokens[t].Substring(peq + 1));
                }
                results.Add(new AuthResult(method.Trim(), result.Trim(), properties));
            }

            parsed = new AuthenticationResults(idTokens[0], results);
            return true;
        }

        /// <summary>Every Authentication-Results header in a raw header block that can be parsed.</summary>
        public static IReadOnlyList<AuthenticationResults> FromHeaders(string rawHeaders)
        {
            var all = new List<AuthenticationResults>();
            foreach (string value in InternetHeaders.GetAll(rawHeaders, MpcConstants.HeaderAuthenticationResults))
            {
                AuthenticationResults parsed;
                if (TryParse(value, out parsed)) all.Add(parsed);
            }
            return all;
        }

        private static string RemoveComments(string s)
        {
            var sb = new StringBuilder(s.Length);
            int depth = 0;
            bool quoted = false;
            for (int i = 0; i < s.Length; i++)
            {
                char c = s[i];
                if (c == '\\' && i + 1 < s.Length)
                {
                    if (depth == 0) sb.Append(c).Append(s[i + 1]);
                    i++;
                    continue;
                }
                if (depth == 0 && c == '"') quoted = !quoted;
                if (!quoted && c == '(') { depth++; continue; }
                if (!quoted && c == ')' && depth > 0) { depth--; sb.Append(' '); continue; }
                if (depth == 0) sb.Append(c);
            }
            return sb.ToString();
        }

        private static List<string> SplitOutsideQuotes(string s, char separator)
        {
            var parts = new List<string>();
            var current = new StringBuilder();
            bool quoted = false;
            for (int i = 0; i < s.Length; i++)
            {
                char c = s[i];
                if (c == '\\' && i + 1 < s.Length) { current.Append(c).Append(s[++i]); continue; }
                if (c == '"') quoted = !quoted;
                if (c == separator && !quoted) { parts.Add(current.ToString()); current.Clear(); continue; }
                current.Append(c);
            }
            parts.Add(current.ToString());
            return parts;
        }

        private static List<string> Tokens(string s)
        {
            var tokens = new List<string>();
            var current = new StringBuilder();
            bool quoted = false;
            foreach (char c in s)
            {
                if (c == '"') quoted = !quoted;
                if (!quoted && char.IsWhiteSpace(c))
                {
                    if (current.Length > 0) { tokens.Add(current.ToString()); current.Clear(); }
                    continue;
                }
                current.Append(c);
            }
            if (current.Length > 0) tokens.Add(current.ToString());
            return tokens;
        }

        private static string Unquote(string s)
        {
            s = s.Trim();
            if (s.Length >= 2 && s[0] == '"' && s[s.Length - 1] == '"') s = s.Substring(1, s.Length - 2).Replace("\\\"", "\"").Replace("\\\\", "\\");
            return s;
        }
    }

    /// <summary>Whether a message's chunks passed the sender checks, and if not, why.</summary>
    public sealed class SenderCheckResult
    {
        internal SenderCheckResult(IReadOnlyList<string> problems)
        {
            Problems = problems;
        }

        public bool Passed
        {
            get { return Problems.Count == 0; }
        }

        /// <summary>Why the message failed, one line per problem; empty when it passed.</summary>
        public IReadOnlyList<string> Problems { get; private set; }
    }

    /// <summary>What <see cref="SenderChecks.SelectPassing{T}"/> chose for one message.</summary>
    public sealed class SenderCheckSelection<T>
    {
        internal SenderCheckSelection(ChunkGroup<T> group, IReadOnlyList<ReceivedChunk<T>> suspicious, IReadOnlyList<string> suspiciousDetails)
        {
            Group = group;
            Suspicious = suspicious;
            SuspiciousDetails = suspiciousDetails;
        }

        /// <summary>The message with a passing copy of each chunk where there is one; duplicates
        /// that passed are its extras.</summary>
        public ChunkGroup<T> Group { get; private set; }

        /// <summary>Duplicates that failed the sender checks or have a different From: address.
        /// Keep them; don't delete them with the rest.</summary>
        public IReadOnlyList<ReceivedChunk<T>> Suspicious { get; private set; }

        /// <summary>Why, one line per suspicious duplicate (same order).</summary>
        public IReadOnlyList<string> SuspiciousDetails { get; private set; }
    }

    /// <summary>
    /// Stage 1 of malware detection (sender and connection checks), on the receiving side.
    /// <para>
    /// Each chunk is checked where it enters the recipient's MPC providers: the relay named in its
    /// X-MPC-Via runs SPF, DKIM and DMARC and records the results in an Authentication-Results
    /// header named after itself, and the recipient's mail server removes any it didn't get from
    /// those relays. A message is rejoined only when, for every chunk, the results from its
    /// relay say SPF passed, a DKIM signature from the From: domain verified and DMARC passed,
    /// with no other result for those three; its From: address is its envelope sender
    /// (Return-Path); and all chunks have the same From: address. A chunk without results from its
    /// relay fails: unchecked is not trusted.
    /// </para>
    /// </summary>
    public static class SenderChecks
    {
        public static SenderCheckResult Evaluate<T>(ChunkGroup<T> group)
        {
            if (group == null) throw new ArgumentNullException("group");

            var problems = new List<string>();
            var fromAddresses = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (ReceivedChunk<T> chunk in group.Chunks)
            {
                problems.AddRange(CheckChunk(chunk, ChunkLabel(chunk)));
                string fromAddress = EmailAddress.GetAddress(chunk.From);
                if (fromAddress != null) fromAddresses.Add(fromAddress);
            }

            if (fromAddresses.Count > 1)
            {
                problems.Add("the chunks have different From: addresses (" + string.Join(", ", fromAddresses.OrderBy(a => a, StringComparer.OrdinalIgnoreCase)) + ")");
            }

            return new SenderCheckResult(problems);
        }

        /// <summary>
        /// Checks every copy of each chunk, not only the one <see cref="MpcCoalescer.Group"/> picked.
        /// <para>
        /// Two emails can carry the same X-MPC-ID and chunk number: a genuine chunk resent from
        /// another server (a replay), or a forged one. Where one copy of a chunk passes the sender
        /// checks and another doesn't, the passing copy is used, so a forged copy can't stop the
        /// message being rejoined. Copies that fail, or whose From: address differs from the chunks
        /// used, are returned as <see cref="SenderCheckSelection{T}.Suspicious"/>: the caller keeps
        /// them and reports them instead of deleting them with the rest. Other duplicates stay
        /// extras, as before.
        /// </para>
        /// </summary>
        public static SenderCheckSelection<T> SelectPassing<T>(ChunkGroup<T> group)
        {
            if (group == null) throw new ArgumentNullException("group");

            var used = new List<ReceivedChunk<T>>();
            foreach (ReceivedChunk<T> picked in group.Chunks)
            {
                // The picked copy first, so nothing changes when every copy passes.
                ReceivedChunk<T> passing = new[] { picked }
                    .Concat(group.Extras.Where(e => e.Chunk.Index == picked.Chunk.Index && e.Chunk.Total == picked.Chunk.Total))
                    .FirstOrDefault(c => CheckChunk(c, ChunkLabel(c)).Count == 0);
                used.Add(passing ?? picked);
            }

            var fromAddresses = new HashSet<string>(
                used.Select(c => EmailAddress.GetAddress(c.From)).Where(a => a != null),
                StringComparer.OrdinalIgnoreCase);

            var extras = new List<ReceivedChunk<T>>();
            var suspicious = new List<ReceivedChunk<T>>();
            var details = new List<string>();
            foreach (ReceivedChunk<T> copy in group.Chunks.Concat(group.Extras))
            {
                if (used.Any(u => ReferenceEquals(u, copy))) continue;

                string label = "duplicate of " + ChunkLabel(copy);
                IReadOnlyList<string> problems = CheckChunk(copy, label);
                string fromAddress = EmailAddress.GetAddress(copy.From);
                if (problems.Count == 0 && fromAddress != null && fromAddresses.Count > 0 && !fromAddresses.Contains(fromAddress))
                {
                    problems = new[] { label + ": From: " + fromAddress + " differs from the other chunks" };
                }

                if (problems.Count == 0)
                {
                    extras.Add(copy);
                }
                else
                {
                    suspicious.Add(copy);
                    details.Add(problems[0]);
                }
            }

            return new SenderCheckSelection<T>(
                new ChunkGroup<T>(group.MpcId, group.Total, used, extras), suspicious, details);
        }

        /// <summary>The sender-check problems of one chunk, each starting with <paramref name="label"/>.</summary>
        public static IReadOnlyList<string> CheckChunk<T>(ReceivedChunk<T> chunk, string label)
        {
            if (chunk == null) throw new ArgumentNullException("chunk");

            var problems = new List<string>();
            string fromAddress = EmailAddress.GetAddress(chunk.From);
            string fromDomain = EmailAddress.GetDomain(chunk.From);
            if (fromAddress == null || fromDomain == null)
            {
                problems.Add(label + ": no From: address");
                return problems;
            }

            // The From: address must be the envelope sender. The sender's mail server only lets a
            // user send with their own address as the envelope sender (reject_sender_login_mismatch)
            // and the entry relay checks its domain with SPF, but DKIM signs whatever From: the user
            // wrote: without this, alice could send a chunk "from" ceo@green.mpc that passes.
            string envelopeSender = EmailAddress.GetAddress(chunk.ReturnPath);
            if (envelopeSender == null)
            {
                problems.Add(label + ": no envelope sender (Return-Path)");
            }
            else if (!string.Equals(envelopeSender, fromAddress, StringComparison.OrdinalIgnoreCase))
            {
                problems.Add(label + ": From: " + fromAddress + " is not the envelope sender " + envelopeSender);
            }

            string relay = NormalizeHost(chunk.Via);
            List<AuthResult> results = chunk.AuthenticationResults
                .Where(ar => string.Equals(NormalizeHost(ar.AuthservId), relay, StringComparison.OrdinalIgnoreCase))
                .SelectMany(ar => ar.Results)
                .ToList();
            if (results.Count == 0)
            {
                problems.Add(label + ": no sender-check results from " + chunk.Via);
                return problems;
            }

            // Every SPF, DKIM and DMARC result from the relay must be a pass, and each of the three
            // must be there. The relay records all three for every chunk, so a pass written by
            // the sender next to the relay's real "fail" can't outvote it.
            // SPF: the chunk came straight from a server the sender's domain allows. (A genuine
            // signed chunk resent from another server still passes DKIM; this catches it.)
            Check(problems, label, chunk.Via, results, "spf", "SPF", r => true,
                "SPF {0} (at {1})");
            // DKIM: a valid signature from the From: domain itself, so the chunk came from that
            // domain's mail server unchanged.
            Check(problems, label, chunk.Via, results, "dkim", "DKIM", r => SameDomain(r.GetProperty("header.d"), fromDomain, false),
                "no valid DKIM signature from " + fromDomain + " (at {1})");
            // DMARC: SPF or DKIM passed for, and lines up with, the From: domain.
            Check(problems, label, chunk.Via, results, "dmarc", "DMARC", r => SameDomain(r.GetProperty("header.from"), fromDomain, true),
                "DMARC {0} for " + fromDomain + " (at {1})");
            return problems;
        }

        private static string ChunkLabel<T>(ReceivedChunk<T> chunk)
        {
            return "chunk " + chunk.Chunk.Index.ToString(CultureInfo.InvariantCulture);
        }

        /// <summary>
        /// Adds a problem unless the relay's results for <paramref name="method"/> include a pass
        /// that <paramref name="matches"/>, and nothing else.
        /// </summary>
        private static void Check(List<string> problems, string label, string relay, List<AuthResult> results,
            string method, string name, Func<AuthResult, bool> matches, string missingFormat)
        {
            List<AuthResult> forMethod = results.Where(r => r.Method == method).ToList();
            AuthResult notPassed = forMethod.FirstOrDefault(r => r.Result != "pass");
            if (!forMethod.Any(r => r.Result == "pass" && matches(r)))
            {
                string result = forMethod.Count == 0 ? "was not checked" : notPassed != null ? notPassed.Result : "passed for another domain";
                problems.Add(label + ": " + string.Format(CultureInfo.InvariantCulture, missingFormat, result, relay));
            }
            else if (notPassed != null)
            {
                problems.Add(label + ": " + name + " results disagree (pass and " + notPassed.Result + ") at " + relay);
            }
        }

        private static string NormalizeHost(string host)
        {
            return (host ?? string.Empty).Trim().TrimEnd('.');
        }

        private static bool SameDomain(string value, string domain, bool missingIsOk)
        {
            if (string.IsNullOrWhiteSpace(value)) return missingIsOk;
            return string.Equals(NormalizeHost(value), domain, StringComparison.OrdinalIgnoreCase);
        }
    }
}
