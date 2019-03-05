using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace Ropufu
{
    namespace LeytePond
    {
        /// <summary>
        /// Interaction logic for MainWindow.xaml
        /// </summary>
        public partial class MainWindow : Window
        {
            const String LeftTagKey = "LeftTag";
            const String RightTagKey = "RightTag";

            #region Dependency Property: Weather

            public static DependencyProperty WeatherProperty = DependencyProperty.Register(nameof(MainWindow.Weather), typeof(Bridge.BattleWeather), typeof(MainWindow),
                new PropertyMetadata(Bridge.BattleWeather.None));

            public Bridge.BattleWeather Weather
            {
                get => (Bridge.BattleWeather)this.GetValue(MainWindow.WeatherProperty);
                set => this.SetValue(MainWindow.WeatherProperty, value);
            }

            #endregion

            private Bridge.BlackMarsh blackMarsh = Bridge.BlackMarsh.Instance;
            private UpdateManager updater = null;

            public MainWindow()
            {
                this.updater = new UpdateManager(this);
                this.InitializeComponent();
            }

            private void ShowHelp() => new HelpWindow() { Owner = this }.ShowDialog();

            private void DisplayUnits()
            {
                foreach (var window in App.Current.Windows)
                {
                    if (window is UnitsWindow)
                    {
                        ((Window)window).Focus();
                        return;
                    }
                }
                new UnitsWindow() { Owner = this }.Show();
            }

            private void AddWaveAfter(Object tag, ArmyView waveView = null)
            {
                if (tag.IsNull()) return;

                var leftTag = this.Resources[MainWindow.LeftTagKey];
                var rightTag = this.Resources[MainWindow.RightTagKey];

                var listView = default(ListView);
                if (Object.ReferenceEquals(tag, leftTag)) listView = this.leftWavesView;
                if (Object.ReferenceEquals(tag, rightTag)) listView = this.rightWavesView;
                if (listView.IsNull()) return;

                if (waveView.IsNull()) listView.Items.Add(new Object());
                else
                {
                    var waveIndex = -1;
                    var generator = listView.ItemContainerGenerator;
                    foreach (var item in listView.Items)
                    {
                        ++waveIndex;
                        var container = (ListViewItem)generator.ContainerFromItem(item);
                        var armyView = (ArmyView)container.FindVisualChild(o => o is ArmyView);
                        if (Object.ReferenceEquals(armyView, waveView)) break;
                    }
                    listView.Items.Insert(waveIndex + 1, new Object());
                    //newWaveView.CaptureCursor();
                }
            }

            private void DeleteWave(Object tag, ArmyView waveView = null)
            {
                if (tag.IsNull()) return;

                var leftTag = this.Resources[MainWindow.LeftTagKey];
                var rightTag = this.Resources[MainWindow.RightTagKey];

                var listView = default(ListView);
                if (Object.ReferenceEquals(tag, leftTag)) listView = this.leftWavesView;
                if (Object.ReferenceEquals(tag, rightTag)) listView = this.rightWavesView;
                if (listView.IsNull()) return;

                if (waveView.IsNull()) return;
                if (listView.Items.Count == 1) return;

                var waveIndex = 0;
                var generator = listView.ItemContainerGenerator;
                foreach (var item in listView.Items)
                {
                    var container = (ListViewItem)generator.ContainerFromItem(item);
                    var armyView = (ArmyView)container.FindVisualChild(o => o is ArmyView);
                    if (Object.ReferenceEquals(armyView, waveView)) break;
                    ++waveIndex;
                }
                if (waveIndex < listView.Items.Count) listView.Items.RemoveAt(waveIndex);
            }

            private String BuildWavesString(ListView listView)
            {
                var waveStrings = new List<String>(listView.Items.Count);
                var generator = listView.ItemContainerGenerator;
                foreach (var item in listView.Items)
                {
                    var container = (ListViewItem)generator.ContainerFromItem(item);
                    var armyView = (ArmyView)container.FindVisualChild(o => o is ArmyView);
                    var wave = armyView.Army;
                    if (!wave.IsEmpty) waveStrings.Add(wave.ToString());
                }
                return String.Join(" + ", waveStrings);
            }

            public void ExecuteSim(Boolean isLog)
            {
                var leftWaves = this.BuildWavesString(this.leftWavesView);
                var rightWaves = this.BuildWavesString(this.rightWavesView);

                this.blackMarsh.Execute(leftWaves, rightWaves, this.Weather, isLog);
                new ReportWindow(this.blackMarsh.Report) { Owner = this }.Show();
            }

            public Logger Warnings => App.Warnings;

            #region Lifetime

            protected override void OnClosing(CancelEventArgs e)
            {
                Properties.Settings.Default.Save();
                base.OnClosing(e);
            }

            protected override void OnPreviewKeyDown(KeyEventArgs e)
            {
                switch (e.Key)
                {
                    case Key.F1:
                        this.ShowHelp();
                        e.Handled = true;
                        break;
                    case Key.Enter:
                        var isLog = (e.KeyboardDevice.Modifiers == ModifierKeys.Shift);
                        if (e.KeyboardDevice.Modifiers == ModifierKeys.None || isLog)
                        {
                            this.ExecuteSim(isLog);
                            e.Handled = true;
                        }
                        break;
                }
                base.OnPreviewKeyDown(e);
            }

            #endregion

            #region Event Handlers

            private void WarningsHandler(Object sender, RoutedEventArgs e) => App.Warnings.Unwind(this);

            private void UnitsHandler(Object sender, RoutedEventArgs e) => this.DisplayUnits();

            private void UpdateHandler(Object sender, RoutedEventArgs e) => this.updater.CheckForUpdates();

            private void AddWaveHandler(Object sender, RoutedEventArgs e) => this.AddWaveAfter((sender as FrameworkElement)?.Tag, sender as ArmyView);

            private void DeleteWaveHandler(Object sender, RoutedEventArgs e) => this.DeleteWave((sender as FrameworkElement)?.Tag, sender as ArmyView);

            #endregion
        }
    }
}
