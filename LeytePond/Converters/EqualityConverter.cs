using System;
using System.Globalization;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    public class EqualityConverter : IValueConverter, IMultiValueConverter
    {
        /// <summary>
        /// Checks if <paramref name="value"/> equals <paramref name="parameter"/>.
        /// </summary>
        public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
        {
            if (value.IsNull()) throw new ArgumentNullException(nameof(value));
            if (parameter.IsNull()) throw new ArgumentNullException(nameof(parameter));

            return value.Equals(parameter);
        }

        /// <summary>
        /// Checks if all passed <paramref name="values"/> are equal.
        /// </summary>
        public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
        {
            if (values.IsNull()) throw new ArgumentNullException(nameof(values));
            foreach (var item in values) if (item.IsNull()) return false;

            var previous = values[0];
            for (var i = 1; i < values.Length; ++i)
            {
                var next = values[i];
                if (!next.Equals(previous)) return false;
                previous = next;
            }
            return true;
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture) => throw new NotSupportedException();

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, CultureInfo culture) => throw new NotSupportedException();
    }
}
