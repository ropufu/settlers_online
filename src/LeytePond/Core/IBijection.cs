using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.Aftermath
{
    public interface IBijection<TLeft, TRight>
    {
        IBijection<TRight, TLeft> Inverse { get; }
        Int32 Count { get; }
        TRight this[TLeft key] { get; set; }

        Boolean ContainsLeft(TLeft key);
        Boolean ContainsRight(TRight key);

        Boolean RemoveLeft(TLeft key);
        Boolean RemoveRight(TRight key);

        Boolean TryGetLeft(TLeft key, out TRight value);
        Boolean TryGetRight(TRight key, out TLeft value);


        void Add(Tuple<TLeft, TRight> item);
        void Add(TLeft left, TRight right);
        void Clear();
    }
}
