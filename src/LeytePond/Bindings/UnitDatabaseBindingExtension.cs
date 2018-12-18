using System;
using System.Windows.Data;

namespace Ropufu.LeytePond
{
    class UnitDatabaseBindingExtension : Binding
    {
        public UnitDatabaseBindingExtension()
        {
            this.Initialize();
        }

        public UnitDatabaseBindingExtension(String path)
            : base(path)
        {
            this.Initialize();
        }

        private void Initialize()
        {
            this.Source = App.Map.Units;
            this.Mode = BindingMode.OneWay;
        }
    }
}
