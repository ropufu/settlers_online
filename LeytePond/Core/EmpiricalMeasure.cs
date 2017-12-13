using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.Aftermath
{
    public static class EmpiricalMeasureExtension
    {
        public static Double Mean(this EmpiricalMeasure<Int32> value)
        {
            var sum = default(Double);
            foreach (var item in value) sum += item.Key * item.Value;
            return sum / value.CountObservations;
        }

        public static Double Mean(this EmpiricalMeasure<Int64> value)
        {
            var sum = default(Double);
            foreach (var item in value) sum += item.Key * item.Value;
            return sum / value.CountObservations;
        }

        public static Double Mean(this EmpiricalMeasure<Single> value)
        {
            var sum = default(Double);
            foreach (var item in value) sum += item.Key * item.Value;
            return sum / value.CountObservations;
        }

        public static Double Mean(this EmpiricalMeasure<Double> value)
        {
            var sum = default(Double);
            foreach (var item in value) sum += item.Key * item.Value;
            return sum / value.CountObservations;
        }
    }

    /** Mirrors structural behavior of \c empirical_measure.hpp. */
    public sealed class EmpiricalMeasure<TKey> : IEnumerable<KeyValuePair<TKey, Double>>
        where TKey : struct, IComparable<TKey>
    {
        private SortedDictionary<TKey, Double> data = null;
        private Double count = default(Double);
        private Double maxHeight = default(Double);
        private TKey mostLikelyValue = default(TKey);
        private TKey minimum = default(TKey);
        private TKey maximum = default(TKey);

        public EmpiricalMeasure()
        {
            this.data = new SortedDictionary<TKey, Double>();
        }

        public EmpiricalMeasure(TKey[] keys, Double[] probabilities)
        {
            if (keys.IsNull()) throw new ArgumentNullException(nameof(keys));
            if (probabilities.IsNull()) throw new ArgumentNullException(nameof(probabilities));
            if (keys.Length != probabilities.Length) throw new ArgumentException();

            var n = keys.Length;
            this.data = new SortedDictionary<TKey, Double>();
            for (var i = 0; i < n; ++i) this.data.Add(keys[i], probabilities[i]);
            this.RebuildStatistics();
        }

        public EmpiricalMeasure(IDictionary<TKey, Double> data)
        {
            if (data.IsNull()) throw new ArgumentNullException(nameof(data));

            this.data = new SortedDictionary<TKey, Double>(data);
            this.RebuildStatistics();
        }

        public void Clear()
        {
            this.data.Clear();
            this.count = 0;

            this.maxHeight = 0;
            this.mostLikelyValue = default(TKey);
            this.minimum = default(TKey);
            this.maximum = default(TKey);
        }

        private void RebuildStatistics()
        {
            if (this.data.Count == 0) return;

            var isFirst = true;
            foreach (var pair in this.data)
            {
                if (isFirst)
                {
                    this.maxHeight = pair.Value;
                    this.mostLikelyValue = pair.Key;
                    this.minimum = pair.Key;
                    this.maximum = pair.Key;
                }

                if (this.minimum.CompareTo(pair.Key) > 0) this.minimum = pair.Key;
                if (this.maximum.CompareTo(pair.Key) < 0) this.maximum = pair.Key;
                if (pair.Value > this.maxHeight)
                {
                    this.maxHeight = pair.Value;
                    this.mostLikelyValue = pair.Key;
                }

                this.count += pair.Value;
                isFirst = false;
            }
        }

        public Double CountOccurences(TKey key) => this.data.ContainsKey(key) ? this.data[key] : 0;

        public Double this[TKey key] => this.data.ContainsKey(key) ? (this.data[key] / this.count) : 0;

        public Double Cdf(TKey key)
        {
            if (this.minimum.CompareTo(key) > 0) return 0;
            if (this.maximum.CompareTo(key) < 0) return 1;

            var p = default(Double);
            foreach (var x in this.data)
            {
                if (x.Key.CompareTo(key) > 0) break;
                p += x.Value;
            }
            return p / this.count;
        }

        public TKey Percentile(Double probability)
        {
            if (probability < 0 || probability > 1) throw new ArgumentOutOfRangeException(nameof(probability));
            if (probability == 0) return this.minimum;
            if (probability == 1) return this.maximum;

            probability *= this.count;
            var p = default(Double);
            foreach (var x in this.data)
            {
                p += x.Value;
                if (p >= probability) return x.Key;
            }
            return this.maximum;
        }

        public IEnumerator<KeyValuePair<TKey, Double>> GetEnumerator() => this.data.GetEnumerator();

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => this.data.GetEnumerator();

        public Double CountObservations => this.count;

        public Int32 CountKeys => this.data.Count;

        public TKey Min => this.minimum;

        public TKey Max => this.maximum;

        public Double MaxProbability => this.maxHeight / this.count;

        public Double MostLikelyCount => this.maxHeight;

        public TKey MostLikelyValue => this.mostLikelyValue;

        public override String ToString() => this.ToString(5);

        public String ToString(Int32 printHeight)
        {
            switch (this.data.Count)
            {
                case 0: return "{ }";
                case 1: return $"Always {this.data.First().Key}.";
            }

            var builder = new StringBuilder();
            var isFirst = true;
            foreach (var item in this.data)
            {
                if (isFirst) isFirst = false;
                else builder.Append(", ");
                builder.Append('{').Append($"{item.Key} : {item.Value / this.count}").Append('}');
            }
            return builder.ToString();
        }
    }
}
