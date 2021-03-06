﻿using System;
using System.Windows.Data;

namespace Ropufu.LeytePond
{
    class CampDatabaseBindingExtension : Binding
    {
        public CampDatabaseBindingExtension()
        {
            this.Initialize();
        }

        public CampDatabaseBindingExtension(String path)
            : base(path)
        {
            this.Initialize();
        }

        private void Initialize()
        {
            this.Source = App.Map.Camps;
            this.Mode = BindingMode.OneWay;
        }
    }
}
