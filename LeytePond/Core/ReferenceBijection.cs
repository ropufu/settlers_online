using System;
using System.Collections.Generic;

namespace Ropufu.Aftermath
{
    public class ReferenceBijection<TLeft, TRight> : IBijection<TLeft, TRight>
        where TLeft : class
        where TRight : class
    {
        private List<TLeft> left = new List<TLeft>();
        private List<TRight> right = new List<TRight>();

        private ReferenceBijection(List<TLeft> left, List<TRight> right)
        {
            this.left = left;
            this.right = right;
        }

        public ReferenceBijection()
        {

        }

        public IBijection<TRight, TLeft> Inverse => new ReferenceBijection<TRight, TLeft>(this.right, this.left);

        /// <exception cref="ArgumentNullException"></exception>
        public Boolean ContainsLeft(TLeft key)
        {
            if (key.IsNull()) throw new ArgumentNullException(nameof(key));
            foreach (var item in this.left) if (Object.ReferenceEquals(item, key)) return true;
            return false;
        }
        /// <exception cref="ArgumentNullException"></exception>
        public Boolean ContainsRight(TRight key)
        {
            if (key.IsNull()) throw new ArgumentNullException(nameof(key));
            foreach (var item in this.right) if (Object.ReferenceEquals(item, key)) return true;
            return false;
        }

        /// <exception cref="ArgumentNullException"></exception>
        public Boolean RemoveLeft(TLeft key)
        {
            if (key.IsNull()) throw new ArgumentNullException(nameof(key));
            for (var i = 0; i < this.left.Count; ++i)
            {
                var item = this.left[i];
                if (Object.ReferenceEquals(item, key))
                {
                    this.left.RemoveAt(i);
                    this.right.RemoveAt(i);
                    return true;
                }
            }
            return false;
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ShouldNotHappenException"></exception>
        public Boolean RemoveRight(TRight key)
        {
            if (key.IsNull()) throw new ArgumentNullException(nameof(key));
            for (var i = 0; i < this.right.Count; ++i)
            {
                var item = this.right[i];
                if (Object.ReferenceEquals(item, key))
                {
                    this.left.RemoveAt(i);
                    this.right.RemoveAt(i);
                    return true;
                }
            }
            return false;
        }

        /// <exception cref="ArgumentNullException"></exception>
        public Boolean TryGetLeft(TLeft key, out TRight value)
        {
            if (key.IsNull()) throw new ArgumentNullException(nameof(key));
            value = default(TRight);
            for (var i = 0; i < this.left.Count; ++i)
            {
                var item = this.left[i];
                if (Object.ReferenceEquals(item, key))
                {
                    value = this.right[i];
                    return true;
                }
            }
            return false;
        }

        /// <exception cref="ArgumentNullException"></exception>
        public Boolean TryGetRight(TRight key, out TLeft value)
        {
            if (key.IsNull()) throw new ArgumentNullException(nameof(key));
            value = default(TLeft);
            for (var i = 0; i < this.right.Count; ++i)
            {
                var item = this.right[i];
                if (Object.ReferenceEquals(item, key))
                {
                    value = this.left[i];
                    return true;
                }
            }
            return false;
        }

        public TRight this[TLeft key]
        {
            /// <exception cref="ArgumentNullException"></exception>
            /// <exception cref="KeyNotFoundException"></exception>
            get
            {
                var value = default(TRight);
                if (!this.TryGetLeft(key, out value)) throw new KeyNotFoundException();
                return value;
            }
            /// <exception cref="ArgumentNullException"></exception>
            set
            {
                if (key.IsNull()) throw new ArgumentNullException(nameof(key));
                value = default(TRight);
                for (var i = 0; i < this.left.Count; ++i)
                {
                    var item = this.left[i];
                    if (Object.ReferenceEquals(item, key))
                    {
                        this.right[i] = value;
                        return;
                    }
                }
                this.left.Add(key);
                this.right.Add(value);
            }
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentException"></exception>
        public void Add(Tuple<TLeft, TRight> item) => this.Add(item.Item1, item.Item2);

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentException"></exception>
        public void Add(TLeft left, TRight right)
        {
            if (this.ContainsLeft(left)) throw new ArgumentException();
            if (this.ContainsRight(right)) throw new ArgumentException();

            this.left.Add(left);
            this.right.Add(right);
        }

        public void Clear()
        {
            this.left.Clear();
            this.right.Clear();
        }

        public Int32 Count => this.left.Count;
    }
}
