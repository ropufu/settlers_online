using System;
using System.Collections.Generic;

namespace Ropufu.Aftermath
{
    public class SortedBijection<TLeft, TRight> : IBijection<TLeft, TRight>
        where TLeft : IComparable<TLeft>
        where TRight : IComparable<TRight>
    {
        private SortedDictionary<TLeft, TRight> leftToRight = new SortedDictionary<TLeft, TRight>();
        private SortedDictionary<TRight, TLeft> rightToLeft = new SortedDictionary<TRight, TLeft>();

        private SortedBijection(SortedDictionary<TLeft, TRight> left, SortedDictionary<TRight, TLeft> right)
        {
            this.leftToRight = left;
            this.rightToLeft = right;
        }

        public SortedBijection()
        {

        }

        public IBijection<TRight, TLeft> Inverse => new SortedBijection<TRight, TLeft>(this.rightToLeft, this.leftToRight);

        /// <exception cref="ArgumentNullException"></exception>
        public Boolean ContainsLeft(TLeft key) => this.leftToRight.ContainsKey(key);
        /// <exception cref="ArgumentNullException"></exception>
        public Boolean ContainsRight(TRight key) => this.rightToLeft.ContainsKey(key);

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ShouldNotHappenException"></exception>
        public Boolean RemoveLeft(TLeft key)
        {
            var right = default(TRight);
            if (!this.leftToRight.TryGetValue(key, out right)) return false; // No key present.

            if (!this.rightToLeft.Remove(right)) throw new ShouldNotHappenException();
            return this.leftToRight.Remove(key);
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ShouldNotHappenException"></exception>
        public Boolean RemoveRight(TRight key)
        {
            var left = default(TLeft);
            if (!this.rightToLeft.TryGetValue(key, out left)) return false; // No key present.

            if (!this.leftToRight.Remove(left)) throw new ShouldNotHappenException();
            return this.rightToLeft.Remove(key);
        }

        /// <exception cref="ArgumentNullException"></exception>
        public Boolean TryGetLeft(TLeft key, out TRight value) => this.leftToRight.TryGetValue(key, out value);
        /// <exception cref="ArgumentNullException"></exception>
        public Boolean TryGetRight(TRight key, out TLeft value) => this.rightToLeft.TryGetValue(key, out value);

        public ICollection<TLeft> LeftKeys => this.leftToRight.Keys;
        public ICollection<TRight> RightKeys => this.rightToLeft.Keys;

        public ICollection<TRight> LeftValues => this.leftToRight.Values;
        public ICollection<TLeft> RightValues => this.rightToLeft.Values;

        public TRight this[TLeft key]
        {
            /// <exception cref="ArgumentNullException"></exception>
            /// <exception cref="KeyNotFoundException"></exception>
            get => this.leftToRight[key];
            /// <exception cref="ArgumentNullException"></exception>
            set
            {
                this.leftToRight[key] = value;
                this.rightToLeft[value] = key;
            }
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentException"></exception>
        public void Add(Tuple<TLeft, TRight> item) => this.Add(item.Item1, item.Item2);

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentException"></exception>
        public void Add(TLeft left, TRight right)
        {
            this.leftToRight.Add(left, right);
            this.rightToLeft.Add(right, left);
        }

        public void Clear()
        {
            this.leftToRight.Clear();
            this.rightToLeft.Clear();
        }

        public Int32 Count => this.leftToRight.Count;
    }
}
