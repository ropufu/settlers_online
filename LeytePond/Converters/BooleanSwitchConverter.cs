using System;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    public class BooleanSwitchConverter : IMultiValueConverter
    {
        public Object Convert(Object[] values, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            if (Object.ReferenceEquals(values, null)) throw new ArgumentNullException(nameof(values));
            if (values.Length != 3) throw new ArgumentOutOfRangeException(nameof(values));
            if (!(values[0] is Boolean)) throw new ArgumentOutOfRangeException(nameof(values));

            return ((Boolean)values[0]) ? values[1] : values[2];
        }

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
