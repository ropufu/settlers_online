using System;
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

            private void ShowHelp() => new HelpWindow() { Owner = this }.Show();

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

            private void WarningsHandler(Object sender, RoutedEventArgs e)
            {
                App.Warnings.Unwind(this);
            }

            private void UpdateHandler(Object sender, RoutedEventArgs e)
            {
                /** @todo Use Github API to update \c ./maps and \c ./faces. */
            }
        }
    }
}
