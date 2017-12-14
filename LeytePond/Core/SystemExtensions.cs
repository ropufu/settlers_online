using System;

namespace Ropufu
{
    static class SystemExtensions
    {
        public static Boolean IsNull<T>(this T item) => Object.ReferenceEquals(item, null);

        public static void Clear<T>(this T[] item)
        {
            for (var i = 0; i < item.Length; ++i) item[i] = default(T);
        }
    }
}
