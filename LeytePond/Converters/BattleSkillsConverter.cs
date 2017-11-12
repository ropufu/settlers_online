using Ropufu.LeytePond.Bridge;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class BattleSkillsConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            var skill = (KeyValuePair<BattleSkill, Int32>)value;

            var path = System.IO.Path.GetFullPath(System.IO.Path.Combine(
                Bridge.Config.Instance.SkillsPath,
                $"{skill.Key.ToReadable()}.png"));
            return path;
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
