using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace Ropufu.LeytePond
{
    // @todo Override IsEditable property to false.
    class CampBox : ComboBox
    {
        private void Traverse(List<DependencyObject> controls)
        {
            while (controls.Count > 0)
            {
                var next = new List<DependencyObject>();
                foreach (var item in controls)
                {
                    //if (item is FrameworkElement) names.Add(((FrameworkElement)item).Name + ": " + item.ToString());
                    //else names.Add("no name: " + item.ToString());

                    var count = VisualTreeHelper.GetChildrenCount(item);
                    for (var i = 0; i < count; ++i) next.Add(VisualTreeHelper.GetChild(item, i));

                    // Remove the display of currently selected item.
                    if (item is ContentPresenter)
                    {
                        var presenter = (ContentPresenter)item;
                        presenter.ContentTemplate = new DataTemplate();
                    }

                    // Change the style of toggle button.
                    if (item is ToggleButton)
                    {
                        var button = (ToggleButton)item;
                        button.Background = Brushes.Transparent;
                        button.BorderBrush = Brushes.Transparent;
                        button.BorderThickness = new Thickness();
                        button.Style = this.ToggleButtonStyle;
                    }
                }
                controls = next;
            }
        }

        public Style ToggleButtonStyle { get; set; }

        protected override void OnInitialized(EventArgs e)
        {
            this.ItemContainerGenerator.StatusChanged += this.StatusChangedHandler;
            base.OnInitialized(e);
        }

        private void StatusChangedHandler(Object sender, EventArgs e)
        {
            var generator = this.ItemContainerGenerator;
            if (generator.Status == GeneratorStatus.ContainersGenerated)
            {
                var index = generator.Items.Count - 1;
                var container = generator.ContainerFromIndex(index) as FrameworkElement;
                if (!container.IsNull()) container.Loaded += (s, ee) =>
                {
                    //this.UpdateLayout();
                    var selected = new List<Object>(1) { this.SelectedItem };
                    var args = new SelectionChangedEventArgs(CampBox.SelectionChangedEvent, selected, selected);
                    this.OnSelectionChanged(args);
                };
            }
        }

        public override void OnApplyTemplate()
        {
            this.Traverse(new List<DependencyObject>() { this });
            base.OnApplyTemplate();
        }
    }
}
