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

        #region Dependency Property: ArmyString

        public static DependencyProperty ArmyStringProperty = DependencyProperty.Register(nameof(ArmyView.ArmyString), typeof(String), typeof(ArmyView),
            new PropertyMetadata(null, new PropertyChangedCallback(ArmyView.OnArmyStringChanged), new CoerceValueCallback(ArmyView.CoerceArmyString)));

        private static void OnArmyStringChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (ArmyView)d;
            var newValue = (String)e.NewValue;
            var warnings = (Warnings)sender.Resources[ArmyView.WarningsKey];
            warnings.Clear();
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

        private Boolean doCheckGenerals = false;
        private Boolean doCoerceFactions = false;
        private Boolean isStrict = false;
        private Boolean isPlayerArmy = false;
        private ArmyParser parser = new ArmyParser(String.Empty);
        private UnitGroups armySource = new UnitGroups();
        private Boolean doHold = false;

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

        public Army ToArmy()
        {
            var warnings = (Warnings)this.Resources[ArmyView.WarningsKey];
            warnings.Clear();
            return this.parser.Build(warnings, this.doCheckGenerals, this.doCoerceFactions, this.isStrict);
        }

        private void InitializeArmySource()
        {
            if (this.doHold) return;

            var groupSum = (GroupSum)this.Resources[ArmyView.GroupSumKey];
            groupSum.ClearChildren();
            this.armySource.Clear();
            var army = this.ToArmy();
            foreach (var g in army.Groups) this.armySource.Add(g);
        }

        public ArmyView()
        {
            this.InitializeComponent();
            this.itemView.Items.Clear();
            this.itemView.ItemsSource = this.armySource;

            var groupSum = (GroupSum)this.Resources[ArmyView.GroupSumKey];
            groupSum.PropertyChanged += this.OnGroupSumChanged;
        }

        private void OnGroupSumChanged(Object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(GroupSum.Children))
            {
                this.doHold = true;
                var army = new Army(armySource);
                this.ArmyString = this.isPlayerArmy ? army.ToCompactString() : army.ToString();
                this.doHold = false;
            }
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

        private void WarningsHandler(Object sender, RoutedEventArgs e)
        {
            var warnings = (Warnings)this.Resources[ArmyView.WarningsKey];
            warnings.Unwind();
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
            }
            base.OnPreviewKeyDown(e);
        }
    }
}
