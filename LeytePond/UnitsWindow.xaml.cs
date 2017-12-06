using Ropufu.LeytePond.Bridge;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Ropufu.LeytePond
{
    /// <summary>
    /// Interaction logic for UnitsWindow.xaml
    /// </summary>
    public partial class UnitsWindow : Window
    {
        private Boolean doTakeAll = true;
        private String[] keywords = new String[0];

        private Point maybeDragStart = new Point();

        public UnitsWindow()
        {
            this.InitializeComponent();

            var view = (CollectionView)CollectionViewSource.GetDefaultView(this.itemView.ItemsSource);
            view.Filter = this.UnitFilter;
        }

        private void DownHandler(Object sender, MouseButtonEventArgs e) => this.maybeDragStart = e.GetPosition(null);

        private void MaybeDragHandler(Object sender, MouseEventArgs e)
        {
            if (e.LeftButton != MouseButtonState.Pressed) return;

            // Get the current mouse position
            var position = e.GetPosition(null);
            var shift = position - this.maybeDragStart;
            var doDrag = (Math.Abs(shift.X) > SystemParameters.MinimumHorizontalDragDistance) || (Math.Abs(shift.Y) > SystemParameters.MinimumVerticalDragDistance);

            if (doDrag)
            {
                var viewItem = sender as ListViewItem;
                var unit = (UnitType)this.itemView.ItemContainerGenerator.ItemFromContainer(viewItem);
                var data = new DataObject(typeof(UnitType).FullName, unit);
                DragDrop.DoDragDrop(viewItem, data, DragDropEffects.Move);
            }

        }

        private Boolean UnitFilter(Object item)
        {
            if (this.doTakeAll) return true;

            var unit = (UnitType)item;
            var isMatch = new Boolean[this.keywords.Length];
            foreach (var name in unit.Names)
            {
                for (var i = 0; i < keywords.Length; ++i)
                    if (name.ToLowerInvariant().Contains(keywords[i])) isMatch[i] = true;
            }
            var hasPassed = true;
            for (var i = 0; i < keywords.Length; ++i) if (!isMatch[i]) hasPassed = false;
            return hasPassed;
        }

        private void FilterChanged(Object sender, TextChangedEventArgs e)
        {
            var filter = this.filterBox.Text.ToLowerInvariant().DeepTrim();
            this.doTakeAll = String.IsNullOrWhiteSpace(filter);
            this.keywords = filter.Split(new Char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            CollectionViewSource.GetDefaultView(this.itemView.ItemsSource).Refresh();
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
