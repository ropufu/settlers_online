using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace Ropufu.LeytePond.Converters
{
    /// <summary>
    /// Element of <see cref="CompositeConverter"/>.
    /// </summary>
    class ConverterTerm
    {
        private IValueConverter converter = null;

        // @todo Add proper \c targetType handling.
        public IValueConverter Converter
        {
            get => this.converter;
            set
            {
                if (value.IsNull()) throw new ArgumentNullException(nameof(value));
                converter = value;
            }
        }

        public Object ConverterParameter { get; set; }
    }

    class CompositeConverter : List<ConverterTerm>, IValueConverter, IMultiValueConverter
    {
        public IMultiValueConverter MultiConverter { get; set; }

        private Object Sequence(Object value, Type targetType, CultureInfo culture)
        {
            foreach (var c in this)
            {
                if (c.Converter.IsNull()) throw new InvalidOperationException($"{nameof(c.Converter)} has to be set first.");
                value = c.Converter.Convert(value, targetType, c.ConverterParameter, culture);
            }
            return value;
        }

        public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
        {
            if (this.Count == 0) return value;
            return this.Sequence(value, targetType, culture);
        }

        public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
        {
            if (this.MultiConverter.IsNull()) throw new InvalidOperationException($"{nameof(this.MultiConverter)} has to be set first.");

            var value = this.MultiConverter.Convert(values, targetType, parameter, culture);
            return this.Sequence(value, targetType, culture);
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture) => throw new NotSupportedException();

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, CultureInfo culture) => throw new NotSupportedException();
    }
}
