using System;
using System.Collections;
using System.Globalization;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    public class BooleanSwitchConverter : IValueConverter, IMultiValueConverter
    {
        /// <summary>
        /// Uses <paramref name="parameter"/> as a return type if <paramref name="value"/> is true; otherwise does nothing.
        /// </summary>
        /// <param name="value">Condition.</param>
        /// <param name="parameter">Return value on success.</param>
        public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
        {
            if (!(value is Boolean)) throw new ArgumentOutOfRangeException(nameof(value));

            var flag = (Boolean)value;
            if (parameter is IEnumerable)
            {
                foreach (var item in (IEnumerable)parameter)
                {
                    if (flag) return item; // Return the first term if the flag is set.
                    flag = true; // Otherwise return the second term (if present).
                }
                return Binding.DoNothing;
            }
            return flag ? parameter : Binding.DoNothing;
        }

        /// <summary>
        /// If the first element of <paramref name="values"/> is true, returns the second element; otherwise returns the third.
        /// </summary>
        /// <param name="values">An array of length three: [ Boolean, Object, Objecct ].</param>
        /// <param name="parameter">Ignored.</param>
        public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
        {
            if (values.IsNull()) throw new ArgumentNullException(nameof(values));
            if (values.Length != 3) throw new ArgumentOutOfRangeException(nameof(values));
            if (!(values[0] is Boolean)) throw new ArgumentOutOfRangeException(nameof(values));

            return ((Boolean)values[0]) ? values[1] : values[2];
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture) => throw new NotSupportedException();

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, CultureInfo culture) => throw new NotSupportedException();
    }
}
