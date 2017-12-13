using System;

namespace Ropufu
{
    static class SystemExtensions
    {
        public static Boolean IsNull<T>(this T item) => Object.ReferenceEquals(item, null);
    }
}
