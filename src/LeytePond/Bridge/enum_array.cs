using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c enum_array.hpp. */
    public class EnumArray<TEnum, TValue> : IEnumerable<EnumArray<TEnum, TValue>.EnumArrayItem>, INotifyCollectionChanged
        where TEnum : struct, IConvertible
        where TValue : IComparable<TValue>
    {
        public sealed class EnumArrayItem
        {
            private EnumArray<TEnum, TValue> collection = null;
            private Int32 position = -1;

            public EnumArrayItem(EnumArray<TEnum, TValue> collection, Int32 position)
            {
                if (collection.IsNull()) throw new ArgumentNullException(nameof(collection));
                this.collection = collection;
                this.position = position;
            }

            public TEnum Key => keys[this.position];

            public TValue Value
            {
                get => this.collection.values[this.position];
                set
                {
                    this.collection.values[this.position] = value;
                    this.collection.NotifyReset();
                }
            }
        }

        public sealed class Enumerator : IEnumerator<EnumArrayItem>
        {
            private Int32 position = -1;
            private EnumArray<TEnum, TValue> collection = null;

            public Enumerator(EnumArray<TEnum, TValue> collection)
            {
                if (Object.ReferenceEquals(collection, null)) throw new ArgumentNullException(nameof(collection));
                this.collection = collection;
            }

            public EnumArrayItem Current => new EnumArrayItem(this.collection, this.position);

            Object IEnumerator.Current => this.Current;

            public Boolean MoveNext()
            {
                while (this.position < keys.Length)
                {
                    ++this.position;
                    if (this.position == keys.Length) return false;
                    if (!this.collection.doSkipDefault) return true;

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

        private Boolean doSkipDefault = false;
        private TValue[] values = new TValue[keys.Length];

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        private void NotifyReset() => this.CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));

        public EnumArray()
        {
            if (!typeof(TEnum).IsEnum) throw new TypeInitializationException(typeof(TEnum).FullName, null);
            
            this.values.Clear();
        }

        public EnumArray(EnumArray<TEnum, TValue> other)
        {
            if (!typeof(TEnum).IsEnum) throw new TypeInitializationException(typeof(TEnum).FullName, null);
            if (Object.ReferenceEquals(other, null)) throw new ArgumentNullException(nameof(other));

            this.doSkipDefault = other.doSkipDefault;
            other.values.CopyTo(this.values, 0);
        }

        public void CopyTo(EnumArray<TEnum, TValue> other)
        {
            if (Object.ReferenceEquals(other, null)) throw new ArgumentNullException(nameof(other));

            other.doSkipDefault = this.doSkipDefault;
            this.values.CopyTo(other.values, 0);
            other.NotifyReset();
        }

        public EnumArray(Dictionary<TEnum, TValue> map)
        {
            if (!typeof(TEnum).IsEnum) throw new TypeInitializationException(typeof(TEnum).FullName, null);

            if (Object.ReferenceEquals(map, null)) throw new ArgumentNullException(nameof(map));
            this.values.Clear();
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

        public Boolean IsEmpty
        {
            get
            {
                var x = default(TValue);
                foreach (var value in this.values)
                {
                    if (Object.ReferenceEquals(value, null)) continue;
                    if (value.CompareTo(x) == 0) continue;
                    return false;
                }
                return true;
            }
        }

        public Boolean DoSkipDefault
        {
            get => this.doSkipDefault;
            set => this.doSkipDefault = value;
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

        public void Clear()
        {
            this.values.Clear();
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

        public IEnumerator<EnumArrayItem> GetEnumerator() => new Enumerator(this);

        IEnumerator IEnumerable.GetEnumerator() => this.GetEnumerator();
    }
}
