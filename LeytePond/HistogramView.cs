using Ropufu.LeytePond.Bridge;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Ropufu.LeytePond
{
    public class HistogramView : Control
    {
        class Bar : FrameworkElement
        {
            private const String BackgroundElementName = "background";
            private const String LineElementName = "line";
            private const String BallElementName = "ball";
            private const Double BallRadius = 2.0;

            public static DependencyProperty StrokeProperty = DependencyProperty.Register(nameof(Bar.Stroke), typeof(Brush), typeof(Bar), 
                new PropertyMetadata(null, new PropertyChangedCallback(Bar.OnStrokePropertyChanged)));

            private static void OnStrokePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
            {
                var sender = (Bar)d;
                sender.line.Stroke = (Brush)e.NewValue;
            }

            public Brush Stroke
            {
                get { return (Brush)this.GetValue(Bar.StrokeProperty); }
                set { this.SetValue(Bar.StrokeProperty, value); }
            }

            private Double value;
            private Size desiredBallSizeCache;

            private Rectangle background = new Rectangle() { Name = Bar.BackgroundElementName, Fill = Brushes.White, StrokeThickness = 0.0 };
            private Line line = new Line() { Name = Bar.LineElementName, StrokeThickness = 1.0 };
            private Ellipse ball = new Ellipse() { Name = Bar.BallElementName, Stroke = Brushes.Maroon, StrokeThickness = 1.0, Fill = Brushes.White, Width = 2 * Bar.BallRadius, Height = 2 * Bar.BallRadius };

            static Bar()
            {
                Bar.DefaultStyleKeyProperty.OverrideMetadata(typeof(Bar), new FrameworkPropertyMetadata(typeof(Bar)));
            }

            public Bar(Double value)
            {
                if (value < 0.0 || value > 1.0) throw new ArgumentOutOfRangeException(nameof(value));
                this.value = value;

                this.AddVisualChild(this.background);
                this.AddVisualChild(this.line);
                this.AddVisualChild(this.ball);
            }

            protected override Int32 VisualChildrenCount => 3;

            protected override Visual GetVisualChild(Int32 index)
            {
                switch (index)
                {
                    case 0: return this.background;
                    case 1: return this.line;
                    case 2: return this.ball;
                    default: throw new ArgumentOutOfRangeException(nameof(index));
                }
            }

            protected override Size MeasureOverride(Size availableSize)
            {
                this.ball.Measure(availableSize);
                this.desiredBallSizeCache = this.ball.DesiredSize;

                if (Double.IsInfinity(availableSize.Width)) availableSize.Width = this.desiredBallSizeCache.Width;
                if (Double.IsInfinity(availableSize.Height)) availableSize.Height = this.desiredBallSizeCache.Height;

                this.background.Width = availableSize.Width;
                this.background.Height = availableSize.Height;
                this.background.Measure(availableSize);

                this.line.X1 = availableSize.Width / 2;
                this.line.X2 = availableSize.Width / 2;
                this.line.Y1 = availableSize.Height;
                this.line.Y2 = (1.0 - this.value) * availableSize.Height;
                this.line.Measure(availableSize);

                return availableSize;
            }

            protected override Size ArrangeOverride(Size finalSize)
            {
                this.background.Arrange(new Rect(0, 0, finalSize.Width, finalSize.Height));
                this.line.Arrange(new Rect(0, 0, finalSize.Width, finalSize.Height));
                this.ball.Arrange(new Rect(
                    (finalSize.Width - this.desiredBallSizeCache.Width) / 2,
                    (1.0 - this.value) * finalSize.Height - this.desiredBallSizeCache.Height / 2,
                    this.desiredBallSizeCache.Width,
                    this.desiredBallSizeCache.Height));

                return finalSize;
            }
        }

        private const Double XLabelTopMargin = 2.0;
        private const Double YLabelBottomMargin = 4.0;

        static HistogramView()
        {
            HistogramView.DefaultStyleKeyProperty.OverrideMetadata(typeof(HistogramView), new FrameworkPropertyMetadata(typeof(HistogramView)));
        }

        private EmpiricalMeasure<Int32> histogram;
        private Thickness graphMargin;
        private Bar[] bars;
        private TextBlock[] xLabels, yLabels;
        private List<Visual> children = new List<Visual>();

        public HistogramView()
        {

        }

        protected override Int32 VisualChildrenCount => (this.histogram == null ? 0 : this.children.Count);

        protected override Visual GetVisualChild(Int32 index)
        {
            if (index < 0 || index >= this.children.Count) throw new ArgumentOutOfRangeException(nameof(index));
            return this.children[index];
        }

        // Warning: not a dependency property!
        public EmpiricalMeasure<Int32> Histogram
        {
            get => this.histogram;
            set
            {
                if (Object.ReferenceEquals(this.histogram, value)) return;

                this.histogram = value;
                this.CreateHistogramView();
                this.InvalidateVisual();
            }
        }

        private void CreateHistogramView()
        {
            // Remove visual children.
            foreach (var child in this.children) this.RemoveVisualChild(child);
            this.children.Clear();
            this.bars = null;
            this.xLabels = null;
            this.yLabels = null;

            if (Object.ReferenceEquals(this.histogram, null)) return;

            // Create visual children.
            var maxCount = this.histogram.MostLikelyCount;
            var norm = this.histogram.CountObservations;

            var i = 0;
            this.bars = new Bar[this.histogram.CountKeys];
            this.xLabels = new TextBlock[this.histogram.CountKeys];
            this.yLabels = new TextBlock[this.histogram.CountKeys];
            foreach (var item in this.histogram)
            {
                var p = item.Value / norm;
                var cdf = this.histogram.Cdf(item.Key);

                this.bars[i] = new Bar(item.Value / maxCount)
                {
                    ToolTip = $@"Empirical probabilities:
ε(X = {item.Key}) = {100 * p:f2}%
ε(X ≤ {item.Key}) = {100 * cdf:f2}%
ε(X ≥ {item.Key}) = {100 * (1.0 - cdf + p):f2}%"
                };
                this.xLabels[i] = new TextBlock() { Text = item.Key.ToString(), IsHitTestVisible = false };
                this.yLabels[i] = new TextBlock() { Text = String.Concat((100 * p).ToString("f2"), "%"), IsHitTestVisible = false };
                i++;
            }
            foreach (var item in this.bars) this.children.Add(item);
            foreach (var item in this.xLabels) this.children.Add(item);
            foreach (var item in this.yLabels) this.children.Add(item);

            foreach (var child in this.children) this.AddVisualChild(child);

            var barStyle = new Style(typeof(Bar));
            var trigger = new Trigger() { Property = Bar.IsMouseOverProperty, Value = true };
            barStyle.Setters.Add(new Setter() { Property = Bar.StrokeProperty, Value = Brushes.LightGray });
            trigger.Setters.Add(new Setter() { Property = Bar.StrokeProperty, Value = Brushes.Maroon });
            barStyle.Triggers.Add(trigger);

            foreach (var item in this.bars) item.Style = barStyle;
        }

        protected override Size MeasureOverride(Size constraint)
        {
            var desiredSize = constraint;

            var smallest = Math.Min(constraint.Width, constraint.Height);
            if (Double.IsInfinity(smallest)) desiredSize = new Size(100.0, 100.0);
            else if (Double.IsInfinity(constraint.Width)) desiredSize = new Size(constraint.Height, constraint.Height);
            else if (Double.IsInfinity(constraint.Height)) desiredSize = new Size(constraint.Width, constraint.Width);

            if (this.histogram != null)
            {
                var mostLikely = this.histogram.MostLikelyValue;
                var min = this.histogram.Min;
                var max = this.histogram.Max;
                //var spread = max == min ? 1.0 : (Double)(max - min);

                var margin = new Thickness();
                var i = 0;
                foreach (var item in this.histogram)
                {
                    this.xLabels[i].Measure(desiredSize);
                    this.yLabels[i].Measure(desiredSize);

                    var desiredX = this.xLabels[i].DesiredSize;
                    var desiredY = this.yLabels[i].DesiredSize;

                    margin.Bottom = Math.Max(margin.Bottom, desiredX.Height);
                    if (item.Key == mostLikely) margin.Top = Math.Max(margin.Top, desiredY.Height);
                    if (i == 0) margin.Left = Math.Max(desiredX.Width, desiredY.Width);
                    if (i == this.histogram.CountKeys - 1) margin.Right = Math.Max(desiredX.Width, desiredY.Width);

                    i++;
                }

                this.graphMargin = new Thickness(
                    (margin.Left + margin.Right) / 2,
                    HistogramView.XLabelTopMargin + margin.Top,
                    (margin.Left + margin.Right) / 2,
                    HistogramView.YLabelBottomMargin + margin.Bottom);

                var cellSize = new Size(
                    Math.Max((desiredSize.Width - this.graphMargin.Left - this.graphMargin.Right) / (max - min + 1), 0.0),
                    Math.Max(desiredSize.Height - this.graphMargin.Top - this.graphMargin.Bottom, 0.0));
                Console.WriteLine(cellSize);

                i = 0;
                foreach (var item in this.histogram) this.bars[i++].Measure(cellSize);
            }

            return desiredSize;
        }

        protected override Size ArrangeOverride(Size arrangeBounds)
        {
            if (this.histogram != null)
            {
                var min = this.histogram.Min;
                var maxCount = this.histogram.MostLikelyCount;

                var i = 0;
                foreach (var item in this.histogram)
                {
                    var barSize = this.bars[i].DesiredSize;
                    var xLabelSize = xLabels[i].DesiredSize;
                    var yLabelSize = yLabels[i].DesiredSize;

                    var y = (maxCount - item.Value) / maxCount;
                    var left = (item.Key - min) * barSize.Width;
                    var middle = left + barSize.Width / 2;

                    this.bars[i].Arrange(new Rect(this.graphMargin.Left + left, this.graphMargin.Top, barSize.Width, barSize.Height));

                    this.xLabels[i].Arrange(new Rect(
                        this.graphMargin.Left + middle - xLabelSize.Width / 2,
                        this.graphMargin.Top + barSize.Height + HistogramView.XLabelTopMargin,
                        xLabelSize.Width,
                        xLabelSize.Height));
                    this.yLabels[i].Arrange(new Rect(
                        this.graphMargin.Left + middle - yLabelSize.Width / 2,
                        this.graphMargin.Top + y * barSize.Height - yLabelSize.Height - HistogramView.YLabelBottomMargin,
                        yLabelSize.Width,
                        yLabelSize.Height));
                    i++;
                }
            }

            return arrangeBounds;
        }
    }
}
