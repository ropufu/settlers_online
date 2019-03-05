using Ropufu.LeytePond.Bridge;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Data;
using System.Globalization;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class BattleWeatherConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            var weather = BattleWeather.None;
            if (Object.ReferenceEquals(value.GetType(), typeof(BattleWeather))) weather = (BattleWeather)value;

            var path = System.IO.Path.GetFullPath(System.IO.Path.Combine(
                Bridge.Config.Instance.SkillsPath,
                $"{weather.ToReadable()}.png"));
            return path;
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
