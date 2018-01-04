using System;
using System.Text;

namespace Ropufu
{
    static class SystemExtensions
    {
        public static Boolean IsNull<T>(this T item) where T : class => Object.ReferenceEquals(item, null);

        /// <exception cref="ArgumentNullException"></exception>
        public static void Clear<T>(this T[] collection)
        {
            if (collection.IsNull()) throw new ArgumentNullException(nameof(collection));

            for (var i = 0; i < collection.Length; ++i) collection[i] = default(T);
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentOutOfRangeException"></exception>
        public static String ToHex(this Byte[] bytes, Boolean isUpperCase = false)
        {
            if (bytes.IsNull()) throw new ArgumentNullException(nameof(bytes));

            try
            {
                var result = new StringBuilder(2 * bytes.Length);
                foreach (var b in bytes) result.Append(b.ToString(isUpperCase ? "X2" : "x2"));
                return result.ToString();
            }
            catch (FormatException e) { throw new ShouldNotHappenException(e); }
        }
    }
}
