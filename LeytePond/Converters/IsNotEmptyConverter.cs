using System;
using System.Windows;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class IsNotEmptyConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            if (Object.ReferenceEquals(value, null)) return false;
            return String.IsNullOrWhiteSpace(value.ToString());
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
