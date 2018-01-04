using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu
{
    /// <summary>
    /// Represents errors due to bugs and coding mistakes; typically indicate that an object is in an invalide state.
    /// </summary>
    [Serializable]
    public class ShouldNotHappenException : Exception
    {
        private const String DefaultMessage = "This should not happen.";

        public ShouldNotHappenException()
            : base(ShouldNotHappenException.DefaultMessage)
        {

        }

        public ShouldNotHappenException(Exception innerException)
            : base(ShouldNotHappenException.DefaultMessage, innerException)
        {

        }
    }
}
