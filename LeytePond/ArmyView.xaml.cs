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
        const String ArmySeparator = "+";
        const Int32 SuggestionsCapacity = 5;
        const String GroupSumKey = "GroupSum";
        const String WarningsKey = "Warnings";
        const String ArmySourceKey = "ArmySource";
        const String SuggestionsSourceKey = "SuggestionsSource";

        #region Dependency Property: IsPlayerArmy, IsNonPlayerArmy

        public static DependencyProperty IsPlayerArmyProperty = DependencyProperty.Register(nameof(ArmyView.IsPlayerArmy), typeof(Boolean), typeof(ArmyView),
            new PropertyMetadata(false, new PropertyChangedCallback(ArmyView.OnIsPlayerArmyChanged)));

        private static readonly DependencyPropertyKey IsNonPlayerArmyKey = DependencyProperty.RegisterReadOnly(nameof(ArmyView.IsNonPlayerArmy), typeof(Boolean), typeof(ArmyView),
            new PropertyMetadata(true));

        public static readonly DependencyProperty IsNonPlayerArmyProperty = ArmyView.IsNonPlayerArmyKey.DependencyProperty;

        // @todo Add adventures marked as IsSuggestedAsPlayer to suggestion list.
        private static void OnIsPlayerArmyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var sender = (ArmyView)d;
            var newValue = (Boolean)e.NewValue;
            sender.IsNonPlayerArmy = !newValue;
        }

        public Boolean IsPlayerArmy
        {
            get => (Boolean)this.GetValue(ArmyView.IsPlayerArmyProperty);
            set => this.SetValue(ArmyView.IsPlayerArmyProperty, value);
        }

        public Boolean IsNonPlayerArmy
        {
            get => (Boolean)this.GetValue(ArmyView.IsNonPlayerArmyProperty);
            protected set => this.SetValue(ArmyView.IsNonPlayerArmyKey, value);
        }

        #endregion

        #region Dependency Property: Decorator

        public static DependencyProperty DecoratorProperty = DependencyProperty.Register(nameof(ArmyView.Decorator), typeof(ArmyDecorator), typeof(ArmyView),
            new PropertyMetadata(new ArmyDecorator(), new PropertyChangedCallback(ArmyView.OnDecoratorChanged), new CoerceValueCallback(ArmyView.CoerceDecorator)));

        private static void OnDecoratorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            //var sender = (ArmyView)d;
            //var newValue = (ArmyDecorator)e.NewValue;
        }

        private static Object CoerceDecorator(DependencyObject d, Object baseValue)
        {
            var sender = (ArmyView)d;
            var value = (ArmyDecorator)baseValue;

            if (value.IsNull()) value = new ArmyDecorator();
            return value;
        }

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

            sender.parsers.Clear();
            var armyStrings = newValue.Split(new String[] { ArmyView.ArmySeparator }, StringSplitOptions.RemoveEmptyEntries);
            foreach (var item in armyStrings) sender.parsers.Add(new ArmyParser(item));

            sender.warnings.Clear();
            sender.InitializeArmySource();
        }

        private static Object CoerceArmyString(DependencyObject d, Object baseValue)
        {
            var sender = (ArmyView)d;
            var value = (String)baseValue;

            if (value.IsNull()) value = String.Empty;
            return value;
        }

        public String ArmyString
        {
            get => (String)this.GetValue(ArmyView.ArmyStringProperty);
            set => this.SetValue(ArmyView.ArmyStringProperty, value);
        }

        #endregion

        private readonly GroupSum groupSum = null;
        private readonly Logger warnings = null;
        private readonly Army armySource = null;
        private readonly StringCollection suggestionsSource = null;

        private Boolean doCheckGenerals = false;
        private Boolean doCoerceFactions = false;
        private Boolean isStrict = false;
        private List<ArmyParser> parsers = new List<ArmyParser>();
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

        private void AppendSuggestions<T>(PrefixDatabase<T> source)
        {
            // ~~ Update suggestions ~~
            var countSuggestions = 0;
            foreach (var s in source.Suggestions)
            {
                this.suggestionsSource.Add(s);
                ++countSuggestions;
                if (countSuggestions == ArmyView.SuggestionsCapacity) break;
            }
        }

        private void InitializeArmySource()
        {
            if (this.doHold) return;

            this.warnings.Clear();
            this.groupSum.ClearChildren();
            this.suggestionsSource.Clear();

            var groups = new List<UnitGroup>();
            foreach (var parser in this.parsers)
            {
                var warnings = new Logger();
                var army = parser.Build(warnings, this.doCheckGenerals, this.doCoerceFactions, this.isStrict);

                if (parser.IsGood) // Take suggestions if string format is that of an army.
                {
                    // ...unless the army has been reconstructed successfully and(!) there is only one (obvious) suggestion.
                    if (army.IsEmpty || UnitDatabase.Instance.Suggestions.Count() > 1) this.AppendSuggestions(UnitDatabase.Instance); 
                }
                else // Otherwise try interpreting the string as adventure name.
                {
                    var adventure = default(Adventure);
                    if (AdventureDatabase.Instance.TryFind(parser.Value, ref adventure))
                    {
                        warnings.Clear();
                        army = new Army((from u in adventure.Units select new UnitGroup(u, 0)).ToArray());
                    }
                    this.AppendSuggestions(AdventureDatabase.Instance);
                }
                // @todo Change the handling of warnings, e.g., missing general check for multiple armies.
                this.warnings.Append(warnings);

                foreach (var g in army)
                {
                    var t = g.Unit;
                    // Try to find the same unit type in existing groups.
                    var hasFound = false;
                    foreach (var x in groups)
                        if (x.Unit == t)
                        {
                            hasFound = true;
                            // Match found: update the count.
                            x.Count += g.Count;
                            break;
                        }

                    // If none exist, add the new group to the list.
                    if (!hasFound) groups.Add(g);
                }
            }

            var newArmy = new Army(groups);
            this.Decorator?.Decorate(newArmy, new Logger());
            newArmy.Skills.DoSkipDefault = true;

            newArmy.CopyTo(this.armySource);

            if (this.suggestionsSource.Count == 0) this.HideSuggestionList();
            else this.ShowSuggestionList();
        }

        private void InitializeArmyString()
        {
            this.doHold = true;
            this.ArmyString = this.IsPlayerArmy ? this.armySource.ToCompactString() : this.armySource.ToString();
            this.Decorator?.Decorate(this.armySource, new Logger());
            this.suggestionsSource.Clear();
            this.HideSuggestionList();
            this.doHold = false;
        }

        public void DeleteSelected()
        {
            // @todo This is horrible!! Rebuild the whole thing just to delete one group?!
            var groups = new List<UnitGroup>(this.armySource.Groups);
            this.groupSum.ClearChildren();

            //var generator = this.itemView.ItemContainerGenerator;
            foreach (UnitGroup g in this.itemView.SelectedItems) groups.Remove(g);

            var newArmy = new Army(groups);
            this.Decorator?.Decorate(newArmy, new Logger());
            newArmy.Skills.DoSkipDefault = true;

            newArmy.CopyTo(this.armySource);
            this.itemView.Items.Refresh();

            this.InitializeArmyString();
        }

        public void ToggleLink()
        {
            if (this.IsNonPlayerArmy) return;

            var generator = this.itemView.ItemContainerGenerator;
            foreach (UnitGroup g in this.itemView.SelectedItems)
            {
                var container = generator.ContainerFromItem(g);
                var groupCountControl = (GroupCountUpDown)container.FindVisualChild(o => o is GroupCountUpDown);
                if (!groupCountControl.IsNull()) groupCountControl.IsCoupled = !groupCountControl.IsCoupled;
            }
        }

        public ArmyView()
        {
            this.InitializeComponent();

            this.groupSum = (GroupSum)this.Resources[ArmyView.GroupSumKey];
            this.warnings = (Logger)this.Resources[ArmyView.WarningsKey];
            this.armySource = (Army)this.Resources[ArmyView.ArmySourceKey];
            this.suggestionsSource = (StringCollection)this.Resources[ArmyView.SuggestionsSourceKey];

            this.groupSum.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(GroupSum.Children))
                {
                    this.InitializeArmyString();
                }
            };

            this.itemView.Items.Clear();
            this.itemView.ItemsSource = this.armySource;
        }

        public void CaptureCursor()
        {
            Keyboard.ClearFocus();

            var scope = FocusManager.GetFocusScope(this.armyStringBox);
            FocusManager.SetFocusedElement(scope, this.armyStringBox);
            Mouse.Capture(this.armyStringBox);
            Keyboard.Focus(this.armyStringBox);

            //this.Focus();
            //this.armyStringBox.Focus();
        }

        private void ShowSkills()
        {
            var general = default(UnitType);
            foreach (var g in this.armySource) if (g.Unit.Is(UnitFaction.General)) general = g.Unit;

            new SkillsWindow(this.Decorator, general) { Owner = App.Current.MainWindow }.ShowDialog();
        }

        private void ShowCamps() => this.campList.IsDropDownOpen = true;

        private void ShowSuggestionList() => this.suggestionList.IsDropDownOpen = true;

        private void HideSuggestionList() => this.suggestionList.IsDropDownOpen = false;

        private void ShowAdventureList() => this.adventureList.IsDropDownOpen = true;

        private void HideAdventureList() => this.adventureList.IsDropDownOpen = false;

        private void ReloadAdventure()
        {
            var adventure = this.adventureList.SelectedItem as Adventure;
            if (adventure.IsNull()) return;

            this.ArmyString = adventure.Name;
        }

        private void WarningsHandler(Object sender, RoutedEventArgs e) => this.warnings.Unwind();

        private void SkillsHandler(Object sender, RoutedEventArgs e) => this.ShowSkills();

        private void CampsHandler(Object sender, RoutedEventArgs e) => this.ShowCamps();

        private void AdventureSelectedHandler(Object sender, SelectionChangedEventArgs e) => this.ReloadAdventure();

        private void ArmyBoxDoubleClickHandler(Object sender, MouseButtonEventArgs e)
        {
            if (String.IsNullOrWhiteSpace(this.ArmyString)) this.ShowAdventureList();
        }

        private void DragEnterHandler(Object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(typeof(List<UnitType>).FullName) || sender == e.Source) e.Effects = DragDropEffects.None;
        }

        private void DropHandler(Object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(List<UnitType>).FullName))
            {
                var isTrivial = true;
                var groups = new List<UnitGroup>(this.armySource.Groups);
                var units = e.Data.GetData(typeof(List<UnitType>).FullName) as List<UnitType>;
                if (units.IsNull()) return;

                foreach (var unit in units)
                {
                    if (this.armySource.Has(u => u.Id == unit.Id)) continue;
                    groups.Add(new UnitGroup(unit, 0));
                    isTrivial = false;
                }
                if (isTrivial) return;

                var newArmy = new Army(groups);
                this.Decorator?.Decorate(newArmy, new Logger());
                newArmy.Skills.DoSkipDefault = true;

                newArmy.CopyTo(this.armySource);
            }
        }

        private void ListItemKeyHandler(Object sender, KeyEventArgs e)
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
                case Key.Delete:
                    if (e.KeyboardDevice.Modifiers == ModifierKeys.None)
                    {
                        if (!(e.KeyboardDevice.FocusedElement is TextBox))
                        {
                            this.DeleteSelected();
                            e.Handled = true;
                        }
                    }
                    break;
            }
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
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
                case Key.Down:
                    if (e.OriginalSource == this.armyStringBox)
                    {
                        this.ShowAdventureList();
                        e.Handled = true;
                    }
                    break;
                case Key.Escape:
                    this.HideAdventureList();
                    this.HideSuggestionList();
                    break;
            }
            base.OnPreviewKeyDown(e);
        }
    }
}
