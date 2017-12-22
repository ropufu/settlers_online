using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Windows;

namespace Ropufu.LeytePond
{
    public class Logger : INotifyPropertyChanged
    {
        private Queue<String> messages = new Queue<String>();

        public event PropertyChangedEventHandler PropertyChanged;

        public Logger()
        {

        }

        public void Unwind(Window owner = null)
        {
            if (Object.ReferenceEquals(owner, null)) owner = App.Current.MainWindow;

            var builder = new StringBuilder();
            while (!this.IsEmpty) builder.AppendLine(this.Pop());
            MessageBox.Show(owner, builder.ToString(), "~~ Oh no! ~~", MessageBoxButton.OK, MessageBoxImage.Warning);
        }

        public Int32 Count => this.messages.Count;
        public Boolean IsEmpty => this.messages.Count == 0;
        public Boolean IsNotEmpty => this.messages.Count != 0;

        public void Clear()
        {
            if (this.messages.Count == 0) return;
            this.messages.Clear();
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
        }

        public void Append(Logger other)
        {
            if (other.IsEmpty) return;
            var wasEmpty = this.IsEmpty;

            foreach (var item in other.messages) this.messages.Enqueue(item);

            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            if (wasEmpty)
            {
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
            }
        }

        public void Push(String message)
        {
            this.messages.Enqueue(message ?? String.Empty);
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            if (this.messages.Count == 1)
            {
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
            }
        }

        public String Pop()
        {
            if (this.messages.Count == 0) return null;

            var message = this.messages.Dequeue();
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            if (this.messages.Count == 0)
            {
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
            }

            return message;
        }

        public String Top => this.messages.Count == 0 ? null : this.messages.Peek();
    }
}
