using Ropufu.Aftermath;
using Ropufu.LeytePond.Bridge;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Ropufu.LeytePond
{
    /// <summary>
    /// Interaction logic for ReportWindow.xaml
    /// </summary>
    public partial class ReportWindow : Window
    {
        private IBijection<EmpiricalMeasure<Int32>, TabItem> histogramTabs = new ReferenceBijection<EmpiricalMeasure<Int32>, TabItem>();

        public ReportWindow()
        {
            this.InitializeComponent();
        }

        public ReportWindow(Report report)
        {
            this.InitializeComponent();
            this.Prepare(report);
        }

        public void Prepare(Report report, Stopwatch timer = null)
        {
            if (Object.ReferenceEquals(report, null)) throw new ArgumentNullException(nameof(report));

            this.reportView.Items.Clear();
            //this.Title = $"Report: {leftName} vs. {rightName}";

            foreach (var entry in report.Entries)
            {
                entry.BuildHistogram();
                if (entry.IsHeader)
                {
                    var entryElement = new TextBlock();
                    if (!String.IsNullOrEmpty(entry.Caption)) entryElement.Inlines.Add(new TextBlock() { Text = entry.Caption, FontWeight = FontWeights.Bold, Margin = new Thickness(5.0) });
                    if (!String.IsNullOrEmpty(entry.Details)) entryElement.Inlines.Add(new TextBlock() { Text = entry.Details, Margin = new Thickness(5.0) });
                    entry.CustomUI = entryElement;
                }
                this.reportView.Items.Add(entry);
            }
        }

        private void ItemPreviewKeyDownHandler(Object sender, KeyEventArgs e)
        {
            var item = sender as ListViewItem;
            var reportEntry = item?.DataContext as ReportEntry;
            if (Object.ReferenceEquals(reportEntry, null)) return;

            switch (e.Key)
            {
                case Key.C:
                    if (e.KeyboardDevice.Modifiers == ModifierKeys.Control)
                    {
                        if (!String.IsNullOrEmpty(reportEntry.ClipboardText)) Clipboard.SetText(reportEntry.ClipboardText);
                        e.Handled = true;
                    }
                    break;
                case Key.Enter:
                    this.TryShowHistogram(reportEntry);
                    e.Handled = true;
                    break;
            }
        }

        private void ItemMouseDoubleClickHandler(Object sender, MouseButtonEventArgs e)
        {
            var reportEntry = (sender as FrameworkElement)?.DataContext as ReportEntry;
            if (Object.ReferenceEquals(reportEntry, null)) return;

            this.TryShowHistogram(reportEntry);
        }

        private void HistogramClickHandler(Object sender, RoutedEventArgs e)
        {
            var reportEntry = (sender as FrameworkElement)?.DataContext as ReportEntry;
            if (Object.ReferenceEquals(reportEntry, null)) return;

            this.TryShowHistogram(reportEntry);
        }

        private Boolean TryShowHistogram(ReportEntry reportEntry)
        {
            if (Object.ReferenceEquals(reportEntry, null)) throw new ArgumentNullException(nameof(reportEntry));

            if (!reportEntry.HasHistogram) return false;
            if (!reportEntry.HasNonDegenerateHistogram) return false;
            this.ShowHistogram(reportEntry.Histogram, reportEntry.Caption);
            return true;
        }

        private void ShowHistogram(EmpiricalMeasure<Int32> histogram, String caption)
        {
            if (Object.ReferenceEquals(histogram, null)) return;

            var tab = default(TabItem);
            if (this.histogramTabs.ContainsLeft(histogram)) tab = this.histogramTabs[histogram];
            else
            {
                tab = new TabItem();
                tab.Header = String.Concat("Histogram: ", caption);
                tab.Content = new HistogramView()
                {
                    Margin = new Thickness(40, 40, 40, 50),
                    BorderThickness = new Thickness(1.0),
                    BorderBrush = Brushes.Beige,
                    Histogram = histogram
                };
                this.reportTabs.Items.Add(tab);
                this.histogramTabs.Add(histogram, tab);

                tab.MouseDoubleClick += (ss, ee) =>
                {
                    this.summaryTab.IsSelected = true;
                    this.histogramTabs.RemoveLeft(histogram);
                    this.reportTabs.Items.Remove(tab);
                };
            }
            tab.IsSelected = true;
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Escape:
                    if (this.summaryTab.IsSelected) this.Close();
                    else
                    {
                        var selectedTab = this.reportTabs.SelectedItem as TabItem;
                        if (selectedTab == null) return;
                        this.summaryTab.IsSelected = true;
                        this.histogramTabs.RemoveRight(selectedTab);
                        this.reportTabs.Items.Remove(selectedTab);
                    }
                    e.Handled = true;
                    break;
            }
            base.OnPreviewKeyDown(e);
        }
    }
}
