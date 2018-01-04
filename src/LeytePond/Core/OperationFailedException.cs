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
    public class OperationFailedException : Exception
    {
        private const String DefaultMessage = "Operation failed.";

        public OperationFailedException()
            : base(OperationFailedException.DefaultMessage)
        {

        }

        public OperationFailedException(Exception innerException)
            : base(OperationFailedException.DefaultMessage, innerException)
        {

        }

        public OperationFailedException(String message)
            : base(message)
        {

        }

        public OperationFailedException(String message, Exception innerException)
            : base(message, innerException)
        {

        }
    }
}
