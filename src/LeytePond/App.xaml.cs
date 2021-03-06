﻿using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Ropufu
{
    namespace LeytePond
    {
        /// <summary>
        /// Interaction logic for App.xaml
        /// </summary>
        public partial class App : Application
        {
            private Logger warnings = new Logger();
            private Logger suggestions = new Logger();
            private Bridge.Map map = new Bridge.Map();

            public static Logger Warnings => App.Current.warnings;
            public static Logger Suggestions => App.Current.suggestions;

            public static Bridge.Map Map => App.Current.map;

            public static new App Current => (App)Application.Current;

            protected override void OnStartup(StartupEventArgs e)
            {
                var blackMarshExe = LeytePond.Properties.Settings.Default.BlackMarshPath;
                var blackMarshConfig = System.IO.Path.ChangeExtension(blackMarshExe, ".config");
                var blackMarshCbor = System.IO.Path.ChangeExtension(blackMarshExe, ".cbor");

                Bridge.BlackMarsh.Instance.ProcessPath = blackMarshExe;
                Bridge.BlackMarsh.Instance.CborPath = blackMarshCbor;
                Bridge.Config.Read(blackMarshConfig);
                this.map = Bridge.Map.LoadFromFolder(Bridge.Config.Instance.MapsPath);

                this.CheckImages();
                base.OnStartup(e);
            }

            private void CheckImages()
            {
                try
                {
                    var facesPath = System.IO.Path.GetFullPath(Bridge.Config.Instance.FacesPath);
                    if (!System.IO.Directory.Exists(facesPath))
                    {
                        this.warnings.Push($"Invalid location for unit faces.");
                        return;
                    }
                    foreach (var unit in this.map.Units.All)
                    {
                        var imagePath = System.IO.Path.Combine(facesPath, $"{unit.FirstName}.png");
                        if (!System.IO.File.Exists(imagePath))
                        {
                            this.warnings.Push($"Missing face for {unit.FirstName}.");
                        }
                    }
                    foreach (var imagePath in System.IO.Directory.GetFiles(facesPath))
                    {
                        var unitName = System.IO.Path.GetFileNameWithoutExtension(imagePath);
                        //var unit = default(Bridge.UnitType);
                        //if (!this.map.Units.TryFind(unitName, ref unit, null))
                        var unit = this.map.Units.Find(unitName, null);
                        if (unit == default(Bridge.UnitType))
                        {
                            this.warnings.Push($"Unrecognized face: {unitName}.");
                        }
                    }
                }
                catch (System.Security.SecurityException)
                {
                    this.warnings.Push($"Encountered security exception when trying to read {Bridge.Config.Instance.FacesPath}.");
                }
                catch (System.IO.PathTooLongException)
                {
                    this.warnings.Push($"Path too long: {Bridge.Config.Instance.FacesPath}.");
                }
            }
        }
    }
}
