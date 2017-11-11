using System;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c char_string.hpp. */
    static class MoreString
    {
        public static String DeepTrim(this String value)
        {
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
    }
}
