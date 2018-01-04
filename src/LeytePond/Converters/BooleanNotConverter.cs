using System;
using System.Windows;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class BooleanNotConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            if (Object.ReferenceEquals(value, null)) throw new ArgumentNullException(nameof(value));
            if (!(value is Boolean)) throw new NotSupportedException();
            return !(Boolean)value;
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            if (Object.ReferenceEquals(value, null)) throw new ArgumentNullException(nameof(value));
            if (!(value is Boolean)) throw new NotSupportedException();
            return !(Boolean)value;
        }
    }
}
