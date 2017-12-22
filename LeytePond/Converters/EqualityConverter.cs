using System;
using System.Globalization;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    /// <summary>
    /// Checks if all the passed arguments are equal.
    /// </summary>
    public class EqualityConverter : IMultiValueConverter
    {
        public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
        {
            if (values.IsNull()) throw new ArgumentNullException(nameof(values));

            var previous = values[0];
            for (var i = 1; i < values.Length; ++i)
            {
                var next = values[i];
                if (next != previous) return false;
                previous = next;
            }
            return true;
        }

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
