using System;
using System.Windows.Data;

namespace Ropufu.LeytePond
{
    class SettingsBindingExtension : Binding
    {
        public SettingsBindingExtension()
        {
            this.Initialize();
        }

        public SettingsBindingExtension(String path)
            : base(path)
        {
            this.Initialize();
        }

        private void Initialize()
        {
            this.Source = Properties.Settings.Default;
            this.Mode = BindingMode.TwoWay;
        }
    }
}
