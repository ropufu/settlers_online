using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace Ropufu.UpdaterApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            this.InitializeComponent();

            var app = App.Current;
            if (!app.IsGood) return;

            this.progressBar.Maximum = (Double)app.CountInstructions;
            app.InstructionExecuting += (sender, e) =>
            {
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(() => 
                {
                    this.actionBlock.Text = "Executing:";
                    this.detailsBlock.Text = e.Instruction.ToString();
                }));
            };
            app.InstructionCompleted += (sender, e) =>
            {
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(() => { ++this.progressBar.Value; }));
            };
            app.InstructionRollback += (sender, e) =>
            {
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(() => 
                {
                    this.actionBlock.Text = "Rolling back:";
                    --this.progressBar.Value;
                }));
            };

            var updaterThread = new Thread(() => this.RunUpdaterThread()) { IsBackground = true };
            try
            {
                updaterThread.Start();
            }
            catch (ThreadStateException) { }
            catch (OutOfMemoryException) { }
        }

        private void RunUpdaterThread()
        {
            var app = App.Current;
            var isGood = app.Execute();
            this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(() =>
            {
                if (!isGood) MessageBox.Show(this, "We could not complete the updating process.", "Update Failed", MessageBoxButton.OK, MessageBoxImage.Error);
                app.RestartServer();
                app.Shutdown();
            }));
        }
    }
}
