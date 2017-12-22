using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu
{
    public static class StringExtensions
    {
        public static String DeepTrim(this String value)
        {
            if (value.IsNull()) return String.Empty;
            var builder = new StringBuilder();
            var wasSpace = false;
            foreach (var c in value)
            {
                var isSpace = Char.IsWhiteSpace(c);
                if (wasSpace && isSpace) continue; // Skip repeated whitespaces.
                wasSpace = isSpace;
                builder.Append(isSpace ? ' ' : c); // Replace whitespaces with ' '.
            }
            return builder.ToString().Trim();
        }

        public static String RelaxCase(this String value) => value.ToLowerInvariant();

        /// <summary>
        /// To be used on trimmed strings.
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        public static String RelaxSpelling(this String value)
        {
            if (value.IsNull()) return String.Empty;
            var relaxed = value.Replace("men", "man");
            if (relaxed.EndsWith("es")) relaxed = relaxed.Substring(0, relaxed.Length - 2);
            else if (relaxed.EndsWith("s")) relaxed = relaxed.Substring(0, relaxed.Length - 1);

            if (relaxed.Length > 4)
            {
                var builder = new StringBuilder();
                var previous = relaxed[0];
                builder.Append(previous);
                foreach (var c in relaxed) if (c != previous) { builder.Append(c); previous = c; }
                relaxed = builder.ToString();
            }

            return relaxed;
        }

        /// <summary>
        /// To be used on trimmed strings.
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        public static String RelaxArticles(this String value)
        {
            var mode = StringComparison.InvariantCultureIgnoreCase;
            var articles = new String[] { "the", "a", "an" };
            foreach (var a in articles)
            {
                var b = a + " ";
                var c = " " + b;
                while (value.StartsWith(b, mode)) value = value.Substring(b.Length);
                value = value.Replace(c, String.Empty);
                //value.IndexOf(a, mode);
            }
            return value;
        }
    }
}
