using Ropufu.LeytePond.Bridge;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class EnumConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            var e = (Enum)value;
            return e.ToString().ToReadable();
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
