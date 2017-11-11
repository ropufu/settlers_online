using System;
using System.Windows;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class UnitImageConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            var path = System.IO.Path.GetFullPath(System.IO.Path.Combine(
                Bridge.Config.Instance.FacesPath,
                $"{value}.png"));
            return path;
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
