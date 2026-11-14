using System;
using System.Collections.Generic;
using System.Text;

namespace Mpc.Core
{
    public static class BodyChunker
    {
        private static readonly Encoding Utf8 = new UTF8Encoding(false);

        /// <summary>
        /// Port of chunkifyEMailBody() in send.c. The UTF-8 body is split into
        /// <paramref name="count"/> pieces of size/count bytes, and the last piece takes the
        /// remainder. Differences from the C code:
        /// a boundary that would fall inside a multi-byte character moves back to the start of
        /// that character, so every chunk is valid text; and exactly <paramref name="count"/>
        /// chunks are always returned (some may be empty), because the receiver needs every chunk
        /// to rejoin the message.
        /// </summary>
        public static IReadOnlyList<string> Split(string body, int count)
        {
            if (count < 1) throw new ArgumentOutOfRangeException("count", "At least one chunk is required.");

            byte[] bytes = Utf8.GetBytes(body ?? string.Empty);
            int size = bytes.Length;
            int chunkSize = size / count;

            var chunks = new List<string>(count);
            int start = 0;
            for (int i = 0; i < count; i++)
            {
                int end;
                if (i == count - 1)
                {
                    end = size; // last chunk takes whatever is left
                }
                else
                {
                    end = Math.Min(size, start + chunkSize);
                    while (end > start && end < size && IsContinuationByte(bytes[end]))
                    {
                        end--;
                    }
                }

                chunks.Add(Utf8.GetString(bytes, start, end - start));
                start = end;
            }

            return chunks;
        }

        private static bool IsContinuationByte(byte b)
        {
            return (b & 0xC0) == 0x80;
        }
    }
}
