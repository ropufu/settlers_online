using System;
using System.Collections.Generic;

namespace Ropufu.UpdaterApp
{
    public class QuietObject
    {
        private Stack<Exception> errors = new Stack<Exception>();

        public Int32 CountErrors => this.errors.Count;

        public Exception LastError => this.errors.Count == 0 ? null : this.errors.Peek();

        public String LastErrorMessage => this.errors.Count == 0 ? null : this.errors.Peek().Message;

        /// <exception cref="ArgumentNullException"></exception>
        protected Boolean OnError(Exception e)
        {
            if (e.IsNull()) throw new ArgumentNullException(nameof(e));
            this.errors.Push(e);
            return false;
        }

        public void ClearErrors() => this.errors.Clear();
    }
}
