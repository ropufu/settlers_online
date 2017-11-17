using Ropufu.LeytePond.Bridge;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Data;
using System.Globalization;

namespace Ropufu.LeytePond.Converters
{
    [Localizability(LocalizationCategory.NeverLocalize)]
    public class BattleSkillsConverter : IValueConverter
    {
        public Object Convert(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            var skill = BattleSkill.None;
            if (Object.ReferenceEquals(value.GetType(), typeof(BattleSkill))) skill = (BattleSkill)value;
            else if (Object.ReferenceEquals(value.GetType(), typeof(EnumArray<BattleSkill, Int32>.EnumArrayItem))) skill = ((EnumArray<BattleSkill, Int32>.EnumArrayItem)value).Key;

            var path = System.IO.Path.GetFullPath(System.IO.Path.Combine(
                Bridge.Config.Instance.SkillsPath,
                $"{skill.ToReadable()}.png"));
            return path;
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    [Localizability(LocalizationCategory.NeverLocalize)]
    public class SkillMapConverter : IMultiValueConverter
    {
        public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
        {
            if (values.IsNull()) return null; // throw new ArgumentNullException(nameof(values));
            if (values.Length != 2) return null; // throw new ArgumentOutOfRangeException(nameof(values));

            var map = values[0] as SkillMap;
            var unit = values[1] as UnitType;

            if (map.IsNull()) return null; // throw new ArgumentException();
            if (unit.IsNull()) return null; // throw new ArgumentException();
            
            return map[unit.FirstName];
        }

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
