﻿using System;
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
            this.Source = Bridge.AdventureDatabase.Instance;
            this.Mode = BindingMode.OneWay;
        }
    }
}
