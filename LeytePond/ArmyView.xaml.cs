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
using System.Windows.Navigation;
using System.Windows.Shapes;
using Ropufu.LeytePond.Bridge;
using System.Collections.ObjectModel;

namespace Ropufu.LeytePond
{
    /// <summary>
    /// Interaction logic for ArmyView.xaml
    /// </summary>
    public partial class ArmyView : UserControl
    {
        const String GroupSumKey = "GroupSum";
        const String WarningsKey = "Warnings";
        const String ArmySourceKey = "ArmySource";

        #region Dependency Property: Decorator

        public static DependencyProperty DecoratorProperty = DependencyProperty.Register(nameof(ArmyView.Decorator), typeof(ArmyDecorator), typeof(ArmyView),
            new PropertyMetadata(new ArmyDecorator()));

        public ArmyDecorator Decorator
        {
            get => (ArmyDecorator)this.GetValue(ArmyView.DecoratorProperty);
            set => this.SetValue(ArmyView.DecoratorProperty, value);
        }

        #endregion

        #region Dependency Property: ArmyString

        public static DependencyProperty ArmyStringProperty = DependencyProperty.Register(nameof(ArmyView.ArmyString), typeof(String), typeof(ArmyView),
            new PropertyMetadata(null, new PropertyChangedCallback(ArmyView.OnArmyStringChanged), new CoerceValueCallback(ArmyView.CoerceArmyString)));

        private static void OnArmyStringChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (ArmyView)d;
            var newValue = (String)e.NewValue;

            sender.warnings.Clear();
            sender.parser = new ArmyParser(newValue);
            sender.InitializeArmySource();
        }

        private static Object CoerceArmyString(DependencyObject d, Object baseValue)
        {
            var sender = (ArmyView)d;
            var value = (String)baseValue;

            if (Object.ReferenceEquals(value, null)) value = String.Empty;
            return value;
        }

        public String ArmyString
        {
            get => (String)this.GetValue(ArmyView.ArmyStringProperty);
            set => this.SetValue(ArmyView.ArmyStringProperty, value);
        }

        #endregion

        private readonly GroupSum groupSum = null;
        private readonly Warnings warnings = null;
        private readonly Army armySource = null;

        private Boolean doCheckGenerals = false;
        private Boolean doCoerceFactions = false;
        private Boolean isStrict = false;
        private Boolean isPlayerArmy = false;
        private ArmyParser parser = new ArmyParser(String.Empty);
        private Boolean doHold = false;

        public event RoutedEventHandler DeleteWave, AddWave;

        public Army Army => this.armySource;
        
        public Boolean DoCheckGenerals
        {
            get => this.doCheckGenerals;
            set => this.doCheckGenerals = value;
        }

        public Boolean DoCoerceFactions
        {
            get => this.doCoerceFactions;
            set => this.doCoerceFactions = value;
        }

        public Boolean IsStrict
        {
            get => this.isStrict;
            set => this.isStrict = value;
        }

        public Boolean IsPlayerArmy
        {
            get => this.isPlayerArmy;
            set => this.isPlayerArmy = value;
        }

        private void InitializeArmySource()
        {
            if (this.doHold) return;
            
            this.warnings.Clear();
            this.groupSum.ClearChildren();

            var newArmy = this.parser.Build(this.warnings, this.doCheckGenerals, this.doCoerceFactions, this.isStrict);
            this.Decorator?.Decorate(newArmy, new Warnings());
            newArmy.Skills.DoSkipDefault = true;

            newArmy.CopyTo(this.armySource);
        }

        public ArmyView()
        {
            this.InitializeComponent();

            this.armySource = (Army)this.Resources[ArmyView.ArmySourceKey];
            this.warnings = (Warnings)this.Resources[ArmyView.WarningsKey];
            this.groupSum = (GroupSum)this.Resources[ArmyView.GroupSumKey];

            this.groupSum.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(GroupSum.Children))
                {
                    this.doHold = true;
                    this.ArmyString = this.isPlayerArmy ? this.armySource.ToCompactString() : this.armySource.ToString();
                    this.doHold = false;
                }
            };

            this.itemView.Items.Clear();
            this.itemView.ItemsSource = this.armySource;
        }

        public void CaptureCursor()
        {
            Keyboard.ClearFocus();

            var scope = FocusManager.GetFocusScope(this.armyTextBox);
            FocusManager.SetFocusedElement(scope, this.armyTextBox);
            Mouse.Capture(this.armyTextBox);
            Keyboard.Focus(this.armyTextBox);

            //this.Focus();
            //this.armyTextBox.Focus();
        }

        public void ToggleLink()
        {
            if (!this.isPlayerArmy) return;

            var generator = this.itemView.ItemContainerGenerator;
            foreach (UnitGroup g in this.itemView.SelectedItems)
            {
                var container = generator.ContainerFromItem(g);
                var groupCountControl = (GroupCountUpDown)container.FindVisualChild(o => o is GroupCountUpDown);
                if (!Object.ReferenceEquals(groupCountControl, null)) groupCountControl.IsCoupled = !groupCountControl.IsCoupled;
            }
        }

        private void ShowSkills()
        {
            var general = default(UnitType);
            foreach (var g in this.armySource) if (g.Unit.Is(UnitFaction.General)) general = g.Unit;

            new SkillsWindow(this.Decorator, general) { Owner = App.Current.MainWindow }.ShowDialog();
        }

        private void WarningsHandler(Object sender, RoutedEventArgs e) => this.warnings.Unwind();

        private void SkillsHandler(Object sender, RoutedEventArgs e) => this.ShowSkills();

        private void DragEnterHandler(Object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(typeof(UnitType).FullName) || sender == e.Source) e.Effects = DragDropEffects.None;
        }

        private void DropHandler(Object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(UnitType).FullName))
            {
                var unit = e.Data.GetData(typeof(UnitType).FullName) as UnitType;
                if (this.armySource.Has(u => u.Id == unit.Id)) return;
                var groups = new List<UnitGroup>(this.armySource.Groups);
                groups.Add(new UnitGroup(unit, 0));

                var newArmy = new Army(groups);
                this.Decorator?.Decorate(newArmy, new Warnings());
                newArmy.Skills.DoSkipDefault = true;

                newArmy.CopyTo(this.armySource);
            }
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.L:
                    if (e.KeyboardDevice.Modifiers == ModifierKeys.None)
                    {
                        if (!(e.KeyboardDevice.FocusedElement is TextBox))
                        {
                            this.ToggleLink();
                            e.Handled = true;
                        }
                    }
                    break;
                case Key.V:
                    if (e.KeyboardDevice.Modifiers == ModifierKeys.Control)
                    {
                        this.ArmyString = Clipboard.GetText().DeepTrim();
                        e.Handled = true;
                    }
                    break;
                case Key.Delete:
                    if (e.KeyboardDevice.Modifiers == ModifierKeys.Control)
                    {
                        this.DeleteWave?.Invoke(this, new RoutedEventArgs());
                        e.Handled = true;
                    }
                    break;
                case Key.Enter:
                    if (e.KeyboardDevice.Modifiers == ModifierKeys.Control)
                    {
                        this.AddWave?.Invoke(this, new RoutedEventArgs());
                        e.Handled = true;
                    }
                    break;
            }
            base.OnPreviewKeyDown(e);
        }
    }
}
