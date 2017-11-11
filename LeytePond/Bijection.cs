using System;
using System.Collections.Generic;

namespace Ropufu.LeytePond
{
    public class Bijection<T, U>
    {
        private SortedDictionary<T, U> leftToRight = new SortedDictionary<T, U>();
        private SortedDictionary<U, T> rightToLeft = new SortedDictionary<U, T>();

        private Bijection(SortedDictionary<T, U> left, SortedDictionary<U, T> right)
        {
            this.leftToRight = left;
            this.rightToLeft = right;
        }

        public Bijection()
        {

        }

        public Bijection<U, T> Inverse => new Bijection<U, T>(this.rightToLeft, this.leftToRight);

        public Boolean ContainsLeft(T key) => this.leftToRight.ContainsKey(key);
        public Boolean ContainsRight(U key) => this.rightToLeft.ContainsKey(key);

        public Boolean RemoveLeft(T key)
        {
            U right;
            if (!this.leftToRight.TryGetValue(key, out right)) return false;
            if (!this.rightToLeft.Remove(right)) return false;
            return this.leftToRight.Remove(key);
        }

        public Boolean RemoveRight(U key)
        {
            T left;
            if (!this.rightToLeft.TryGetValue(key, out left)) return false;
            if (!this.leftToRight.Remove(left)) return false;
            return this.rightToLeft.Remove(key);
        }

        public Boolean TryGetLeft(T key, out U value) => this.leftToRight.TryGetValue(key, out value);
        public Boolean TryGetRight(U key, out T value) => this.rightToLeft.TryGetValue(key, out value);

        public ICollection<T> LeftKeys => this.leftToRight.Keys;
        public ICollection<U> RightKeys => this.rightToLeft.Keys;

        public ICollection<U> LeftValues => this.leftToRight.Values;
        public ICollection<T> RightValues => this.rightToLeft.Values;

        public U this[T key]
        {
            get => this.leftToRight[key];
            set
            {
                this.leftToRight[key] = value;
                this.rightToLeft[value] = key;
            }
        }
        
        public void Add(Tuple<T, U> item) => this.Add(item.Item1, item.Item2);

        public void Add(T left, U right)
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
