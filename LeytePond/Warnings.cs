using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Windows;

namespace Ropufu.LeytePond
{
    public class Warnings : INotifyPropertyChanged
    {
        private Queue<String> stack = new Queue<String>();

        public event PropertyChangedEventHandler PropertyChanged;

        public Warnings()
        {

        }

        public void Unwind(Window owner = null)
        {
            if (Object.ReferenceEquals(owner, null)) owner = App.Current.MainWindow;

            var builder = new StringBuilder();
            while (!this.IsEmpty) builder.AppendLine(this.Pop());
            MessageBox.Show(owner, builder.ToString(), "~~ Oh no! ~~", MessageBoxButton.OK, MessageBoxImage.Warning);
        }

        public Int32 Count => this.stack.Count;
        public Boolean IsEmpty => this.stack.Count == 0;
        public Boolean IsNotEmpty => this.stack.Count != 0;

        public void Clear()
        {
            if (this.stack.Count == 0) return;
            this.stack.Clear();
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
        }

        public void Push(String message)
        {
            this.stack.Enqueue(message ?? String.Empty);
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            if (this.stack.Count == 1)
            {
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
            }
        }

        public String Pop()
        {
            if (this.stack.Count == 0) return null;

            var message = this.stack.Dequeue();
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            if (this.stack.Count == 0)
            {
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsNotEmpty)));
            }

            return message;
        }

        public String Top => this.stack.Count == 0 ? null : this.stack.Peek();
    }
}
