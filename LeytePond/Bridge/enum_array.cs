using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c enum_array.hpp. */
    public class EnumArray<TEnum, TValue> : IEnumerable<KeyValuePair<TEnum, TValue>>, INotifyCollectionChanged
        where TEnum : struct, IConvertible
        where TValue : IComparable<TValue>
    {
        public sealed class Enumerator : IEnumerator<KeyValuePair<TEnum, TValue>>
        {
            private Int32 position = -1;
            private EnumArray<TEnum, TValue> collection = null;

            public Enumerator(EnumArray<TEnum, TValue> collection)
            {
                if (Object.ReferenceEquals(collection, null)) throw new ArgumentNullException(nameof(collection));
                this.collection = collection;
            }

            public KeyValuePair<TEnum, TValue> Current => new KeyValuePair<TEnum, TValue>(keys[this.position], this.collection.values[this.position]);

            Object IEnumerator.Current => this.Current;

            public Boolean MoveNext()
            {
                while (this.position < keys.Length)
                {
                    ++this.position;
                    if (this.position == keys.Length) return false;

                    var value = this.collection.values[this.position];
                    if (Object.ReferenceEquals(value, null)) continue;
                    if (value.CompareTo(default(TValue)) == 0) continue;
                    return true;
                }
                return false;
            }

            public void Reset() => this.position = -1;

            #region IDisposable Support
            private Boolean disposedValue = false; // To detect redundant calls

            private void Dispose(Boolean disposing)
            {
                if (!this.disposedValue)
                {
                    this.collection = null;
                    this.disposedValue = true;
                }
            }

            // This code added to correctly implement the disposable pattern.
            public void Dispose() => this.Dispose(true);
            #endregion
        }

        private static TEnum[] keys; // List of possible values of <TEnum>.
        private static SortedDictionary<TEnum, Int32> lookup; // Inverse lookup: indices of <TEnum> in <keys>.

        static EnumArray()
        {
            var knownKeys = Enum.GetValues(typeof(TEnum));
            keys = new TEnum[knownKeys.Length];
            lookup = new SortedDictionary<TEnum, Int32>();
            for (var i = 0; i < keys.Length; i++)
            {
                keys[i] = (TEnum)knownKeys.GetValue(i);
                //if (lookup.ContainsKey(keys[i])) throw new TypeInitializationException(typeof(TEnum).FullName, null);
                lookup.Add(keys[i], i);
            }
        }

        private TValue[] values = new TValue[keys.Length];

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        private void NotifyReset() => this.CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));

        public EnumArray()
        {
            if (!typeof(TEnum).IsEnum) throw new TypeInitializationException(typeof(TEnum).FullName, null);

            this.values.Initialize();
        }

        public EnumArray(EnumArray<TEnum, TValue> other)
        {
            if (!typeof(TEnum).IsEnum) throw new TypeInitializationException(typeof(TEnum).FullName, null);
            if (Object.ReferenceEquals(other, null)) throw new ArgumentNullException(nameof(other));

            other.values.CopyTo(this.values, 0);
        }

        public void CopyTo(EnumArray<TEnum, TValue> other)
        {
            if (Object.ReferenceEquals(other, null)) throw new ArgumentNullException(nameof(other));
            this.values.CopyTo(other.values, 0);
            other.NotifyReset();
        }

        public EnumArray(Dictionary<TEnum, TValue> map)
        {
            if (!typeof(TEnum).IsEnum) throw new TypeInitializationException(typeof(TEnum).FullName, null);

            if (Object.ReferenceEquals(map, null)) throw new ArgumentNullException(nameof(map));
            this.values.Initialize();
            for (var i = 0; i < keys.Length; i++)
            {
                var maybe = default(TValue);
                if (map.TryGetValue(keys[i], out maybe)) this.values[i] = maybe;
            }
        }

        public static EnumArray<TEnum, TValue> Parse(Dictionary<String, TValue> map)
        {
            if (Object.ReferenceEquals(map, null)) throw new ArgumentNullException(nameof(map));
            var result = new EnumArray<TEnum, TValue>();
            foreach (var pair in map)
            {
                result[pair.Key.CppParse<TEnum>()] = pair.Value;
            }
            return result;
        }

        public TValue this[TEnum key]
        {
            get => this.values[lookup[key]];
            set
            {
                this.values[lookup[key]] = value;
                this.NotifyReset();
            }
        }

        public void Initialize()
        {
            this.values.Initialize();
            this.NotifyReset();
        }

        public override String ToString()
        {
            var jsonPairs = new List<String>(keys.Length);
            for (var i = 0; i < keys.Length; i++)
            {
                var x = this.values[i];
                if (x.Equals(default(TValue))) continue;
                jsonPairs.Add($"\"{keys[i].ToString().ToCpp()}\": {x}");
            }
            return String.Concat("{ ", String.Join(", ", jsonPairs), " }");
        }

        public override Int32 GetHashCode()
        {
            var result = 1;
            unchecked
            {
                foreach (var x in this.values)
                {
                    if (!x.Equals(default(TValue))) result ^= x.GetHashCode();
                    result <<= 1;
                }
            }
            return result;
        }

        public IEnumerator<KeyValuePair<TEnum, TValue>> GetEnumerator() => new Enumerator(this);

        IEnumerator IEnumerable.GetEnumerator() => this.GetEnumerator();
    }
}
