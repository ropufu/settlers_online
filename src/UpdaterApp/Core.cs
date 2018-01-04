using System;
using System.IO;

namespace Ropufu.UpdaterApp
{
    internal static class Core
    {
        public static Boolean IsNull<T>(this T value) where T : class => Object.ReferenceEquals(value, null);

        public static Boolean TryReadLine(this StreamReader reader, out String value)
        {
            value = null;
            try
            {
                value = reader.ReadLine();
                if (value.IsNull()) return false;
                return true;
            }
            catch (IOException) { return false; }
            catch (OutOfMemoryException) { return false; }
        }

        public static Boolean TryReadInt32(this StreamReader reader, ref Int32 value)
        {
            try
            {
                var line = reader.ReadLine();
                if (line.IsNull()) return false;
                if (!Int32.TryParse(line, out value)) return false;
                return true;
            }
            catch (IOException) { return false; }
            catch (OutOfMemoryException) { return false; }
        }
    }
}
