using System;
using System.Windows.Data;

namespace Ropufu.LeytePond
{
    class AdventureDatabaseBindingExtension : Binding
    {
        public AdventureDatabaseBindingExtension()
        {
            this.Initialize();
        }

        public AdventureDatabaseBindingExtension(String path)
            : base(path)
        {
            this.Initialize();
        }

        private void Initialize()
        {
            this.Source = App.Map.Adventures;
            this.Mode = BindingMode.OneWay;
        }
    }
}
