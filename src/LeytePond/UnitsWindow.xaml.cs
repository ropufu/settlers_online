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
        private List<Adventure> adventures = new List<Adventure>(App.Map.Adventures.All);
        private List<UnitType> oldSelection = new List<UnitType>();

        private Point maybeDragStart = new Point();

        public UnitsWindow()
        {
            this.adventures.Insert(0, new Adventure("All"));
            this.InitializeComponent();

            var view = (CollectionView)CollectionViewSource.GetDefaultView(this.itemView.ItemsSource);
            view.Filter = this.UnitFilter;
        }

        public IEnumerable<Adventure> Adventures => this.adventures;

        private void CacheSelection()
        {
            this.oldSelection = new List<UnitType>(this.itemView.SelectedItems.Count);
            foreach (UnitType u in this.itemView.SelectedItems) this.oldSelection.Add(u);
        }

        private Boolean WasSelected(Object item)
        {
            if (item.IsNull()) return false;
            if (this.itemView.SelectedItems.IsNull()) return false;

            foreach (var u in this.itemView.SelectedItems) if (Object.ReferenceEquals(item, u)) return true;
            return false;
        }

        private void PreviewDownHandler(Object sender, MouseButtonEventArgs e)
        {
            var generator = this.itemView.ItemContainerGenerator;
            var container = sender as DependencyObject;
            if (generator.IsNull() || container.IsNull()) return;

            var unit = generator.ItemFromContainer(container);

            // If the item clicked on was already selected, then cache the existing selection, because it will be deselected by the time drag kicks in.
            if (this.WasSelected(unit)) this.CacheSelection();
            else this.oldSelection.Clear(); // It the item clicked on was not selected, the deselection is intentional, and the cache should be cleared.

            this.maybeDragStart = e.GetPosition(null);
        }

        private void MaybeDragHandler(Object sender, MouseEventArgs e)
        {
            if (e.LeftButton != MouseButtonState.Pressed) return;

            // Get the current mouse position.
            var position = e.GetPosition(null);
            var shift = position - this.maybeDragStart;
            var doDrag = (Math.Abs(shift.X) > SystemParameters.MinimumHorizontalDragDistance) || (Math.Abs(shift.Y) > SystemParameters.MinimumVerticalDragDistance);

            if (doDrag)
            {
                if (this.oldSelection.IsNull()) throw new ShouldNotHappenException();
                var viewItem = sender as ListViewItem;
                if (this.oldSelection.Count == 0) this.CacheSelection(); // If the cache is empty, rebuild it with the current selection.
                var data = new DataObject(typeof(List<UnitType>).FullName, this.oldSelection);
                DragDrop.DoDragDrop(viewItem, data, DragDropEffects.Move);
            }

        }

        private Boolean UnitFilter(Object item)
        {
            var unit = (UnitType)item;

            // Primary filter: by adventure.
            if (this.adventuresBox.SelectedIndex != 0)
            {
                var adventure = (Adventure)this.adventuresBox.SelectedValue;
                if (!adventure.Has(unit)) return false;
            }

            // Secondary filter: by text.
            if (this.doTakeAll) return true;

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

        private void FilterChangedHandler(Object sender, TextChangedEventArgs e)
        {
            var filter = this.filterBox.Text.ToLowerInvariant().DeepTrim();
            this.doTakeAll = String.IsNullOrWhiteSpace(filter);
            this.keywords = filter.Split(new Char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            if (this.itemView.IsNull()) return;
            CollectionViewSource.GetDefaultView(this.itemView.ItemsSource).Refresh();
        }

        private void AdventureChangedHandler(Object sender, SelectionChangedEventArgs e)
        {
            if (this.itemView.IsNull()) return;
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
