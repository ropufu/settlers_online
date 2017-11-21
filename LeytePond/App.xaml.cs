using System;
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
            private Warnings warnings = new Warnings();

            public static Warnings Warnings => App.Current.warnings;

            public static new App Current => (App)Application.Current;

            protected override void OnStartup(StartupEventArgs e)
            {
                var blackMarshExe = LeytePond.Properties.Settings.Default.BlackMarshPath;
                var blackMarshConfig = System.IO.Path.ChangeExtension(blackMarshExe, ".config");
                var blackMarshCbor = System.IO.Path.ChangeExtension(blackMarshExe, ".cbor");

                Bridge.BlackMarsh.Instance.ProcessPath = blackMarshExe;
                Bridge.BlackMarsh.Instance.CborPath = blackMarshCbor;
                Bridge.Config.Read(blackMarshConfig);
                Bridge.UnitDatabase.Instance.LoadFromFolder(Bridge.Config.Instance.MapsPath);

                this.CheckImages();
                base.OnStartup(e);
            }

            public void SyncMaps(List<GitHubFileInfo> maps, out List<GitHubFileInfo> updates)
            {
                updates = new List<GitHubFileInfo>(maps.Count);
                try
                {
                    var mapsPath = System.IO.Path.GetFullPath(Bridge.Config.Instance.MapsPath);
                    if (!System.IO.Directory.Exists(mapsPath))
                    {
                        this.warnings.Push($"Invalid location for maps.");
                        return;
                    }
                    foreach (var mapInfo in maps)
                    {
                        var mapPath = System.IO.Path.Combine(mapsPath, mapInfo.Name);
                        if (!System.IO.File.Exists(mapPath))
                        {
                            this.warnings.Push($"Missing map {mapInfo.Name}.");
                            mapInfo.MarkForUpdate(mapPath, NotifyCollectionChangedAction.Add);
                            updates.Add(mapInfo);
                        }
                        else if (GitHubFileInfo.HashFile(mapPath).ToHex() != mapInfo.Sha)
                        {
                            this.warnings.Push($"Update available for map {mapInfo.Name}.");
                            mapInfo.MarkForUpdate(mapPath, NotifyCollectionChangedAction.Replace);
                            updates.Add(mapInfo);
                        }
                    }
                    foreach (var mapPath in System.IO.Directory.GetFiles(mapsPath))
                    {
                        var mapName = System.IO.Path.GetFileName(mapPath);
                        var isMissing = true;
                        foreach (var mapInfo in maps) if (mapInfo.Name == mapName) isMissing = false;
                        if (isMissing)
                        {
                            this.warnings.Push($"Map deprecated: {mapName}.");
                            var mapInfo = new GitHubFileInfo() { Name = mapName };
                            mapInfo.MarkForUpdate(mapPath, NotifyCollectionChangedAction.Remove);
                            updates.Add(mapInfo);
                        }
                    }
                }
                catch (System.Security.SecurityException)
                {
                    this.warnings.Push($"Encountered security exception when trying to read {Bridge.Config.Instance.MapsPath}.");
                }
                catch (System.IO.PathTooLongException)
                {
                    this.warnings.Push($"Path too long: {Bridge.Config.Instance.MapsPath}.");
                }
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
                    foreach (var unit in Bridge.UnitDatabase.Instance.Units)
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
                        var unit = default(Bridge.UnitType);
                        if (!Bridge.UnitDatabase.Instance.TryFind(unitName, ref unit))
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
