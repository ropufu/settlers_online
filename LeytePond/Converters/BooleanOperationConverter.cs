using System;
using System.Windows;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    public enum BooleanOperation
    {
        And,
        Or,
        Xor
    }

    public class BooleanOperationConverter : IMultiValueConverter
    {
        public Object Convert(Object[] values, Type targetType, Object parameter, System.Globalization.CultureInfo culture)
        {
            if (Object.ReferenceEquals(values, null)) throw new ArgumentNullException(nameof(values));
            if (Object.ReferenceEquals(parameter, null)) throw new ArgumentNullException(nameof(parameter));

            if (values.Length == 0) throw new ArgumentOutOfRangeException(nameof(values), $"Value must contain at least one element.");
            if (Object.ReferenceEquals(values[0], DependencyProperty.UnsetValue)) return null;
            if (!(values[0] is Boolean)) throw new ArgumentOutOfRangeException(nameof(values), $"Value at {0} cannot be cast to <{typeof(Boolean)}>: {values[0]}");
            if (!(parameter is String)) throw new ArgumentOutOfRangeException(nameof(parameter));

            var result = (Boolean)values[0];
            switch (((String)parameter).ToLowerInvariant())
            {
                case "and":
                    for (var i = 1; i < values.Length; i++)
                    {
                        if (Object.ReferenceEquals(values[i], DependencyProperty.UnsetValue)) return null;
                        if (!(values[i] is Boolean)) throw new ArgumentOutOfRangeException(nameof(values), $"Value at {i} cannot be cast to <{typeof(Boolean)}>: {values[i]}");
                        result &= (Boolean)values[i];
                    }
                    return result;
                case "or":
                    for (var i = 1; i < values.Length; i++)
                    {
                        if (Object.ReferenceEquals(values[i], DependencyProperty.UnsetValue)) return null;
                        if (!(values[i] is Boolean)) throw new ArgumentOutOfRangeException(nameof(values), $"Value at {i} cannot be cast to <{typeof(Boolean)}>: {values[i]}");
                        result |= (Boolean)values[i];
                    }
                    return result;
                case "xor":
                    for (var i = 1; i < values.Length; i++)
                    {
                        if (Object.ReferenceEquals(values[i], DependencyProperty.UnsetValue)) return null;
                        if (!(values[i] is Boolean)) throw new ArgumentOutOfRangeException(nameof(values), $"Value at {i} cannot be cast to <{typeof(Boolean)}>: {values[i]}");
                        result ^= (Boolean)values[i];
                    }
                    return result;
                default:
                    throw new ArgumentOutOfRangeException(nameof(parameter));
            }
        }

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
