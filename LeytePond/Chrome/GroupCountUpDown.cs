using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Ropufu.LeytePond
{
    public class GroupCountUpDown : FrameworkElement
    {
        private const String UpArrowPath = "M0,4C0,4 0,6 0,6 0,6 3.5,2.5 3.5,2.5 3.5,2.5 7,6 7,6 7,6 7,4 7,4 7,4 3.5,0.5 3.5,0.5 3.5,0.5 0,4 0,4z";
        private const String DownArrowPath = "M0,2.5C0,2.5 0,0.5 0,0.5 0,0.5 3.5,4 3.5,4 3.5,4 7,0.5 7,0.5 7,0.5 7,2.5 7,2.5 7,2.5 3.5,6 3.5,6 3.5,6 0,2.5 0,2.5z";
        private const String LeftArrowPath = "M3.18,7C3.18,7 5,7 5,7 5,7 1.81,3.5 1.81,3.5 1.81,3.5 5,0 5,0 5,0 3.18,0 3.18,0 3.18,0 0,3.5 0,3.5 0,3.5 3.18,7 3.18,7z";
        private const String RightArrowPath = "M1.81,7C1.81,7 0,7 0,7 0,7 3.18,3.5 3.18,3.5 3.18,3.5 0,0 0,0 0,0 1.81,0 1.81,0 1.81,0 5,3.5 5,3.5 5,3.5 1.81,7 1.81,7z";

        private static Geometry upArrowGeometry = Geometry.Parse(UpArrowPath);
        private static Geometry downArrowGeometry = Geometry.Parse(DownArrowPath);
        private static Geometry leftArrowGeometry = Geometry.Parse(LeftArrowPath);
        private static Geometry rightArrowGeometry = Geometry.Parse(RightArrowPath);

        static GroupCountUpDown()
        {
            GroupCountUpDown.DefaultStyleKeyProperty.OverrideMetadata(typeof(GroupCountUpDown), new FrameworkPropertyMetadata(typeof(GroupCountUpDown)));

            GroupCountUpDown.upArrowGeometry.Freeze();
            GroupCountUpDown.downArrowGeometry.Freeze();
            GroupCountUpDown.leftArrowGeometry.Freeze();
            GroupCountUpDown.rightArrowGeometry.Freeze();
        }

        //    ▴ ▾ ▲ ▼
        //
        //   ---------------------------======
        //   ||                |   ▲   |    ||
        //   || text box       |-------|    ||
        //   |=================|===▼===|======
        //   ---------------------------
        //
        //
        //   ---------------------------
        //   ||                |   ▲|| |
        //   || text box       |-------|
        //   ||                |   ▼|| |
        //   ---------------------------
        //   ||                     ||
        //   =========================
        //    

        private TextBox textControl = new TextBox() { VerticalContentAlignment = VerticalAlignment.Center, TextAlignment = TextAlignment.Right };
        /** @todo Change Margin on up / down controls when orientation changes. */
        private RepeatButton upControl = new RepeatButton() { IsTabStop = false, Content = new Path() { Fill = Brushes.Black, Margin = new Thickness(3.0, 0.5, 3.0, 0.5), Data = GroupCountUpDown.upArrowGeometry } };
        private RepeatButton downControl = new RepeatButton() { IsTabStop = false, Content = new Path() { Fill = Brushes.Black, Margin = new Thickness(3.0, 0.5, 3.0, 0.5), Data = GroupCountUpDown.downArrowGeometry } };

        private Boolean isValueSync = false;
        private GroupSum constraint, groupToRestrict;
        private Int32 smallChange = 1;
        private Int32 largeChange = 10;

        #region Dependency Property: ArrowDock

        public static DependencyProperty ArrowDockProperty = DependencyProperty.Register(nameof(GroupCountUpDown.ArrowDock), typeof(Dock), typeof(GroupCountUpDown),
            new FrameworkPropertyMetadata(Dock.Right, FrameworkPropertyMetadataOptions.AffectsArrange | FrameworkPropertyMetadataOptions.AffectsMeasure));

        public Dock ArrowDock
        {
            get { return (Dock)this.GetValue(GroupCountUpDown.ArrowDockProperty); }
            set { this.SetValue(GroupCountUpDown.ArrowDockProperty, value); }
        }

        #endregion

        #region Dependency Property: Minimum, Maximum

        public static DependencyProperty MinimumProperty = DependencyProperty.Register(nameof(GroupCountUpDown.Minimum), typeof(Int32), typeof(GroupCountUpDown),
            new PropertyMetadata(0, new PropertyChangedCallback(GroupCountUpDown.OnMinimumChanged), new CoerceValueCallback(GroupCountUpDown.CoerceMinimum)));

        public static DependencyProperty MaximumProperty = DependencyProperty.Register(nameof(GroupCountUpDown.Maximum), typeof(Int32), typeof(GroupCountUpDown),
            new PropertyMetadata(Int32.MaxValue, new PropertyChangedCallback(GroupCountUpDown.OnMaximumChanged), new CoerceValueCallback(GroupCountUpDown.CoerceMaximum)));

        private static void OnMinimumChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (GroupCountUpDown)d;
            sender.CheckIfMinimumAttained((Int32)e.NewValue, sender.Value);
        }

        private static void OnMaximumChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (GroupCountUpDown)d;
            sender.CheckIfMaximumAttained((Int32)e.NewValue, sender.Value);
        }

        private static Object CoerceMinimum(DependencyObject d, Object baseValue)
        {
            var sender = (GroupCountUpDown)d;
            var value = (Int32)baseValue;
            if (value > sender.Maximum) value = sender.Maximum;
            return value;
        }

        private static Object CoerceMaximum(DependencyObject d, Object baseValue)
        {
            var sender = (GroupCountUpDown)d;
            var value = (Int32)baseValue;
            if (value < sender.Minimum) value = sender.Minimum;
            return value;
        }

        public Int32 Minimum
        {
            get { return (Int32)this.GetValue(GroupCountUpDown.MinimumProperty); }
            set { this.SetValue(GroupCountUpDown.MinimumProperty, value); }
        }

        public Int32 Maximum
        {
            get { return (Int32)this.GetValue(GroupCountUpDown.MaximumProperty); }
            set { this.SetValue(GroupCountUpDown.MaximumProperty, value); }
        }

        #endregion

        #region Dependency Property: IsValueRequired, Value

        public static DependencyProperty IsValueRequiredProperty = DependencyProperty.Register(nameof(GroupCountUpDown.IsValueRequired), typeof(Boolean), typeof(GroupCountUpDown),
            new PropertyMetadata(false, new PropertyChangedCallback(GroupCountUpDown.OnIsValueRequiredPropertyChanged), new CoerceValueCallback(GroupCountUpDown.CoerceIsValueRequired)));

        public static DependencyProperty ValueProperty = DependencyProperty.Register(nameof(GroupCountUpDown.Value), typeof(Int32?), typeof(GroupCountUpDown),
            new FrameworkPropertyMetadata(new Int32?(), FrameworkPropertyMetadataOptions.AffectsRender, new PropertyChangedCallback(GroupCountUpDown.OnValueChanged), new CoerceValueCallback(GroupCountUpDown.CoerceValue)));

        private static void OnIsValueRequiredPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (GroupCountUpDown)d;
            var isValueRequired = (Boolean)e.NewValue;
            if (isValueRequired && !sender.Value.HasValue) // If value became required, set it to <Minimum>.
            {
                sender.Value = sender.Minimum;
            }
        }

        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (GroupCountUpDown)d;
            var newValue = (Int32?)e.NewValue;
            sender.SyncValue(sender, newValue);

            sender.constraint?.OnValueChanged(sender, (Int32?)e.OldValue, newValue);
            sender.groupToRestrict?.InvalidateConstraint();

            sender.CheckIfMinimumAttained(sender.Minimum, newValue);
            sender.CheckIfMaximumAttained(sender.Maximum, newValue);
        }

        private static Object CoerceValue(DependencyObject d, Object baseValue)
        {
            var sender = (GroupCountUpDown)d;
            return sender.CoerceValue((Int32?)baseValue);
        }

        private static Object CoerceIsValueRequired(DependencyObject d, Object baseValue)
        {
            var sender = (GroupCountUpDown)d;
            if (Object.ReferenceEquals(sender.constraint, null)) return baseValue;
            return true; // When part of constraint group, value is always required.
        }

        private Int32? CoerceValue(Int32? value)
        {
            if (value.HasValue)
            {
                if (value.Value < this.Minimum) value = this.Minimum;
                else if (value.Value > this.Maximum) value = this.Maximum;
            }
            else if (this.IsValueRequired) value = this.Minimum;
            return value;
        }

        public Boolean IsValueRequired
        {
            get { return (Boolean)this.GetValue(GroupCountUpDown.IsValueRequiredProperty); }
            set { this.SetValue(GroupCountUpDown.IsValueRequiredProperty, value); }
        }

        public Int32? Value
        {
            get { return (Int32?)this.GetValue(GroupCountUpDown.ValueProperty); }
            set { this.SetValue(GroupCountUpDown.ValueProperty, this.CoerceValue(value)); }
        }

        #endregion

        #region Dependency Property: IsCoupled

        public static DependencyProperty IsCoupledProperty = DependencyProperty.Register(nameof(GroupCountUpDown.IsCoupled), typeof(Boolean), typeof(GroupCountUpDown),
            new PropertyMetadata(false, new PropertyChangedCallback(GroupCountUpDown.OnIsCoupledChanged), new CoerceValueCallback(GroupCountUpDown.CoerceIsCoupled)));

        private static void OnIsCoupledChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (GroupCountUpDown)d;
            var newValue = (Boolean)e.NewValue;
            sender.constraint?.InvalidateConstraint();
        }

        private static Object CoerceIsCoupled(DependencyObject d, Object baseValue)
        {
            var sender = (GroupCountUpDown)d;
            return sender.CoerceIsCoupled((Boolean)baseValue);
        }

        private Boolean CoerceIsCoupled(Boolean value)
        {
            if (value == false) return value; // Can always remove coupling: coerced value is <false>.
            if (Object.ReferenceEquals(this.constraint, null)) return false;

            if (!this.constraint.CanCouple) return false; // Check if coupling is allowed; if not, coerced value is <false>.
            //if (sender.IsSacrificed) return false; // No coupling for groups marked for sacrifice: coerced value is <false>.
            return value; // Allow coupling: coerced value is <true>.
        }

        public Boolean IsCoupled
        {
            get { return (Boolean)this.GetValue(GroupCountUpDown.IsCoupledProperty); }
            set { this.SetValue(GroupCountUpDown.IsCoupledProperty, this.CoerceIsCoupled(value)); }
        }

        #endregion

        /// <summary>
        /// Synchronize value between this instance and children.
        /// </summary>
        /// <param name="d"></param>
        /// <param name="newValue"></param>
        private void SyncValue(DependencyObject d, Int32? newValue)
        {
            if (this.isValueSync) return;

            this.isValueSync = true;
            if (!Object.ReferenceEquals(d, this)) this.Value = newValue;
            if (!Object.ReferenceEquals(d, this.textControl)) this.textControl.Text = newValue?.ToString() ?? String.Empty;
            this.isValueSync = false;
        }

        private void CheckIfMinimumAttained(Int32 minimum, Int32? value)
        {
            this.downControl.IsEnabled = value.HasValue ? (minimum < value.Value) : true;
        }

        private void CheckIfMaximumAttained(Int32 maximum, Int32? value)
        {
            this.upControl.IsEnabled = value.HasValue ? (maximum > value.Value) : true;
        }

        public GroupCountUpDown()
        {
            this.AddVisualChild(this.textControl);
            this.AddVisualChild(this.upControl);
            this.AddVisualChild(this.downControl);

            this.textControl.TextChanged += new TextChangedEventHandler((sender, e) =>
            {
                var text = this.textControl.Text;
                Int32 newValue;
                if (String.IsNullOrEmpty(text)) this.SyncValue(this.textControl, null);
                else
                {
                    var isTextInvalid = !Int32.TryParse(text, out newValue);
                    if (!isTextInvalid) this.SyncValue(this.textControl, newValue);
                    //else this.SyncValue(this, this.Value);
                }
            });

            //this.textControl.GotKeyboardFocus += new KeyboardFocusChangedEventHandler((sender, e) =>
            //{
            //    this.CaptureMouse();
            //});

            this.textControl.LostKeyboardFocus += new KeyboardFocusChangedEventHandler((sender, e) =>
            {
                this.SyncValue(this, this.Value);
                //this.ReleaseMouseCapture();
            });

            this.upControl.Click += new RoutedEventHandler((sender, e) =>
            {
                this.IncreaseValue(this.smallChange);
            });

            this.downControl.Click += new RoutedEventHandler((sender, e) =>
            {
                this.DecreaseValue(this.smallChange);
            });
        }

        /// <summary>
        /// Constraint object imposed upon this instance.
        /// </summary>
        /// <remarks>
        /// Not a dependency property.
        /// </remarks>
        public GroupSum Constraint
        {
            get { return this.constraint; }
            set
            {
                if (!Object.ReferenceEquals(this.constraint, null)) this.constraint.RemoveChild(this); // Remove current parent-child relationship.

                if (Object.ReferenceEquals(value, null)) this.constraint = value;
                else
                {
                    this.IsValueRequired = true;
                    this.constraint = value;
                    value.AddChild(this); // Add new parent-child relationship.
                }
            }
        }

        /// <summary>
        /// Constraint object that this instance will restrict.
        /// </summary>
        /// <remarks>
        /// Not a dependency property.
        /// </remarks>
        public GroupSum GroupToRestrict
        {
            get { return this.groupToRestrict; }
            set
            {
                if (!Object.ReferenceEquals(this.groupToRestrict, null)) this.groupToRestrict.Master = null; // Remove current parent-child relationship.

                this.groupToRestrict = value;
                if (!Object.ReferenceEquals(value, null)) value.Master = this; // Add new parent-child relationship.
            }
        }

        protected void IncreaseValue(Int32 delta)
        {
            var value = this.Value;
            this.Value = value.HasValue ? value + delta : this.Minimum;
        }

        protected void DecreaseValue(Int32 delta)
        {
            var value = this.Value;
            this.Value = value.HasValue ? value - delta : this.Maximum;
        }

        #region Keyboard / Mouse Events

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Up:
                    this.IncreaseValue(this.smallChange);
                    e.Handled = true;
                    break;
                case Key.Down:
                    this.DecreaseValue(this.smallChange);
                    e.Handled = true;
                    break;
                case Key.PageUp:
                    this.IncreaseValue(this.largeChange);
                    e.Handled = true;
                    break;
                case Key.PageDown:
                    this.DecreaseValue(this.largeChange);
                    e.Handled = true;
                    break;
            }
            base.OnPreviewKeyDown(e);
        }

        //protected override void OnPreviewMouseWheel(MouseWheelEventArgs e)
        //{
        //    if (e.Delta > 0) this.IncreaseValue(e.Delta / this.wheelDeltaDenominator);
        //    else if (e.Delta < 0) this.DecreaseValue((-e.Delta) / this.wheelDeltaDenominator);
        //    e.Handled = true;
        //    base.OnPreviewMouseWheel(e);
        //}

        #endregion

        #region Visual Children

        protected override Int32 VisualChildrenCount => 3;

        protected override Visual GetVisualChild(Int32 index)
        {
            switch (index)
            {
                case 0: return this.textControl;
                case 1: return this.upControl;
                case 2: return this.downControl;
                default: throw new ArgumentOutOfRangeException(nameof(index));
            }
        }

        #endregion

        #region Measure / Arrange

        private Size desiredSizeCache = new Size();
        private Size desiredUpDownSizeCache = new Size();
        private Dock desiredDockCache;

        protected override Size MeasureOverride(Size availableSize)
        {
            this.desiredDockCache = this.ArrowDock;
            var upDownAvailableSize = availableSize;

            switch (this.desiredDockCache)
            {
                case Dock.Left:
                case Dock.Right:
                    ((Path)this.upControl.Content).Data = GroupCountUpDown.upArrowGeometry;
                    ((Path)this.downControl.Content).Data = GroupCountUpDown.downArrowGeometry;
                    upDownAvailableSize.Height /= 2;
                    break;
                case Dock.Bottom:
                case Dock.Top:
                    ((Path)this.upControl.Content).Data = GroupCountUpDown.rightArrowGeometry;
                    ((Path)this.downControl.Content).Data = GroupCountUpDown.leftArrowGeometry;
                    upDownAvailableSize.Width /= 2;
                    break;
            }

            this.textControl.Measure(availableSize);
            this.upControl.Measure(upDownAvailableSize);
            this.downControl.Measure(upDownAvailableSize);

            var desiredTextSize = this.textControl.DesiredSize;
            var desiredUpSize = this.upControl.DesiredSize;
            var desiredDownSize = this.downControl.DesiredSize;

            var width = 0.0;
            var height = 0.0;
            switch (this.desiredDockCache)
            {
                case Dock.Left:
                case Dock.Right:
                    width = desiredTextSize.Width;
                    if (desiredUpSize.Width > width) width = desiredUpSize.Width;
                    if (desiredDownSize.Width > width) width = desiredDownSize.Width;

                    height = desiredUpSize.Height + desiredDownSize.Height;
                    if (desiredTextSize.Height > height) height = desiredTextSize.Height;
                    break;
                case Dock.Bottom:
                case Dock.Top:
                    width = desiredUpSize.Width + desiredDownSize.Width;
                    if (desiredTextSize.Width > width) width = desiredTextSize.Width;

                    height = desiredTextSize.Height;
                    if (desiredUpSize.Height > height) height = desiredUpSize.Height;
                    if (desiredDownSize.Height > height) height = desiredDownSize.Height;
                    break;
            }

            this.desiredUpDownSizeCache.Width = Math.Max(desiredUpSize.Width, desiredDownSize.Width);
            this.desiredUpDownSizeCache.Height = Math.Max(desiredUpSize.Height, desiredDownSize.Height);
            this.desiredSizeCache = new Size(width, height);
            return this.desiredSizeCache;
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            var desiredHeight = this.desiredUpDownSizeCache.Height;
            var desiredWidth = this.desiredUpDownSizeCache.Width;
            var desiredWidthToHeightRatio = desiredWidth / desiredHeight;

            var textBoxRect = Rect.Empty;
            var upButtonRect = Rect.Empty;
            var downButtonRect = Rect.Empty;

            var textBoxDim = 0.0;
            var textBoxOffset = 0.0;
            var upDownOffset = 0.0;

            switch (this.desiredDockCache)
            {
                case Dock.Left:
                case Dock.Right:
                    // Take all the height available.
                    desiredHeight = finalSize.Height;
                    // Check for downscaling.
                    if (desiredHeight < 2 * this.desiredUpDownSizeCache.Height)
                    {
                        desiredWidth = desiredHeight * desiredWidthToHeightRatio;
                    }

                    textBoxDim = Math.Max(0.0, finalSize.Width - desiredWidth);
                    if (this.desiredDockCache == Dock.Left)
                    {
                        textBoxOffset = desiredWidth;
                        upDownOffset = 0.0;
                    }
                    else
                    {
                        textBoxOffset = 0.0;
                        upDownOffset = textBoxDim;
                    }

                    textBoxRect = new Rect(textBoxOffset, 0.0, textBoxDim, desiredHeight);
                    upButtonRect = new Rect(upDownOffset, 0.0, desiredWidth, desiredHeight / 2);
                    downButtonRect = new Rect(upDownOffset, upButtonRect.Height, desiredWidth, desiredHeight / 2);
                    break;
                case Dock.Bottom:
                case Dock.Top:
                    // Take all the width available.
                    desiredWidth = finalSize.Width;
                    // Check for downscaling.
                    if (desiredWidth < 2 * this.desiredUpDownSizeCache.Width)
                    {
                        desiredHeight = desiredWidth / desiredWidthToHeightRatio;
                    }

                    textBoxDim = Math.Max(0.0, finalSize.Height - desiredHeight);
                    if (this.desiredDockCache == Dock.Top)
                    {
                        textBoxOffset = desiredHeight;
                        upDownOffset = 0.0;
                    }
                    else
                    {
                        textBoxOffset = 0.0;
                        upDownOffset = textBoxDim;
                    }

                    textBoxRect = new Rect(0.0, textBoxOffset, desiredWidth, textBoxDim);
                    downButtonRect = new Rect(0.0, upDownOffset, desiredWidth / 2, desiredHeight);
                    upButtonRect = new Rect(downButtonRect.Width, upDownOffset, desiredWidth / 2, desiredHeight);
                    break;
            }

            this.textControl.Arrange(textBoxRect);
            this.upControl.Arrange(upButtonRect);
            this.downControl.Arrange(downButtonRect);

            return finalSize;
        }

        #endregion
    }
}
