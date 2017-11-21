using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Text;
using System.Windows;
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
            private Bridge.BlackMarsh blackMarsh = Bridge.BlackMarsh.Instance;

            public MainWindow()
            {
                this.InitializeComponent();

                this.leftArmyView.Decorator = Bridge.Config.Instance.Left;
                this.rightArmyView.Decorator = Bridge.Config.Instance.Right;
            }

            private void ShowHelp() => new HelpWindow() { Owner = this }.ShowDialog();

            private void DownloadUpdates()
            {
                var mapsUrl = "https://api.github.com/repos/ropufu/settlers_online/contents/maps";
                var maps = GitHubFileInfo.Get(mapsUrl);

                var updates = default(List<GitHubFileInfo>);
                App.Current.SyncMaps(maps, out updates);

                if (updates.Count > 0)
                {
                    var builder = new StringBuilder();
                    builder.AppendLine("The following updates are available:");
                    foreach (var mapFile in updates)
                    {
                        switch (mapFile.UpdateAction)
                        {
                            case NotifyCollectionChangedAction.Add: builder.Append(" + "); break;
                            case NotifyCollectionChangedAction.Remove: builder.Append(" x "); break;
                            default: builder.Append(" - "); break;
                        }
                        builder.AppendLine(mapFile.Name);
                    }
                    builder.AppendLine().AppendLine("Proceed with download?");

                    switch (MessageBox.Show(this, builder.ToString(), "Updates Available", MessageBoxButton.OKCancel, MessageBoxImage.Question))
                    {
                        case MessageBoxResult.OK:
                            foreach (var mapFile in updates) if (!mapFile.Update()) App.Warnings.Push($"Failed to update map {mapFile.Name}.");
                            break;
                    }
                }
            }

            public Warnings Warnings => App.Warnings;

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
                        if (!(e.OriginalSource is System.Windows.Controls.ComboBoxItem))
                        {
                            var isLog = (e.KeyboardDevice.Modifiers == ModifierKeys.Control);
                            var leftArmy = this.leftArmyView.Army;
                            var rightArmy = this.rightArmyView.Army;
                            if (leftArmy.IsEmpty) break;
                            if (rightArmy.IsEmpty) break;

                            this.blackMarsh.Execute(leftArmy.ToString(), rightArmy.ToString(), isLog);
                            new ReportWindow(this.blackMarsh.Report) { Owner = this }.Show();
                            e.Handled = true;
                        }
                        break;
                }
                base.OnPreviewKeyDown(e);
            }

            private void WarningsHandler(Object sender, RoutedEventArgs e) => App.Warnings.Unwind(this);

            private void UpdateHandler(Object sender, RoutedEventArgs e) => this.DownloadUpdates();
        }
    }
}
