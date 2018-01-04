using System;
using System.Windows.Data;

namespace Ropufu.LeytePond
{
    class ConfigBindingExtension : Binding
    {
        public ConfigBindingExtension()
        {
            this.Initialize();
        }

        public ConfigBindingExtension(String path)
            : base(path)
        {
            this.Initialize();
        }

        private void Initialize()
        {
            this.Source = Bridge.Config.Instance;
            this.Mode = BindingMode.TwoWay;
        }
    }
}
