namespace Mpc.Core
{
    public static class EmailAddress
    {
        /// <summary>
        /// Returns the lower-cased domain of an address such as "user@example.com" or
        /// "Some Name &lt;user@example.com&gt;", or null when there is none.
        /// (getDomainFromEmailAddress in send.c takes everything after the first '@'.)
        /// </summary>
        public static string GetDomain(string address)
        {
            if (string.IsNullOrWhiteSpace(address)) return null;

            string s = address.Trim();
            int lt = s.LastIndexOf('<');
            int gt = s.LastIndexOf('>');
            if (lt >= 0 && gt > lt)
            {
                s = s.Substring(lt + 1, gt - lt - 1);
            }

            int at = s.LastIndexOf('@');
            if (at < 0 || at == s.Length - 1) return null;

            string domain = s.Substring(at + 1).Trim().TrimEnd('.', '>').Trim();
            return domain.Length == 0 ? null : domain.ToLowerInvariant();
        }
    }
}
