using System.Windows;
using System.Windows.Input;

namespace Ropufu.LeytePond
{
    /// <summary>
    /// Interaction logic for HelpWindow.xaml
    /// </summary>
    public partial class HelpWindow : Window
    {
        public HelpWindow()
        {
            this.InitializeComponent();
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Escape:
                    this.Close();
                    e.Handled = true;
                    break;
            }
            base.OnPreviewKeyDown(e);
        }
    }
}
