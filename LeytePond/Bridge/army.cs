using System;
using System.Linq;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Collections;

namespace Ropufu.LeytePond.Bridge
{
    public class UnitGroups : ObservableCollection<UnitGroup>
    {
        public event EventHandler ItemChanged;

        private void OnItemChanged(Object sender, PropertyChangedEventArgs e)
        {
            this.ItemChanged?.Invoke(this, new EventArgs());
        }

        protected override void ClearItems()
        {
            foreach (var g in this) g.PropertyChanged -= this.OnItemChanged;
            base.ClearItems();
            this.ItemChanged?.Invoke(this, new EventArgs());
        }

        protected override void InsertItem(Int32 index, UnitGroup item)
        {
            item.PropertyChanged += this.OnItemChanged;

            base.InsertItem(index, item);
            this.ItemChanged?.Invoke(this, new EventArgs());
        }

        protected override void RemoveItem(Int32 index)
        {
            var g = this[index];
            g.PropertyChanged -= this.OnItemChanged;

            base.RemoveItem(index);
            this.ItemChanged?.Invoke(this, new EventArgs());
        }

        protected override void SetItem(Int32 index, UnitGroup item)
        {
            var g = this[index];
            g.PropertyChanged -= this.OnItemChanged;
            item.PropertyChanged += this.OnItemChanged;

            base.SetItem(index, item);
            this.ItemChanged?.Invoke(this, new EventArgs());
        }
    }

    /** Mirrors structural behavior of \c army.hpp. */
    public class Army : IEnumerable<UnitGroup>, INotifyCollectionChanged, INotifyPropertyChanged
    {
        private static readonly String EmptyArmyString = "empty";
        private static readonly Func<UnitType, String> DefaultFormat = u => " " + u.FirstName;
        private static readonly Func<UnitType, String> CompactFormat = u => UnitDatabase.BuildKey(u);

        private Boolean doHoldNotifications = false;
        private UnitGroups groups = new UnitGroups();
        private Camp camp = new Camp();
        private Double frenzyBonus = 0;
        private EnumArray<BattleSkill, Int32> skills = new EnumArray<BattleSkill, Int32>();
        private List<BattleTrait> traits = new List<BattleTrait>();

        public event NotifyCollectionChangedEventHandler CollectionChanged;
        public event PropertyChangedEventHandler PropertyChanged;

        public IEnumerator<UnitGroup> GetEnumerator() => this.groups.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => this.GetEnumerator();

        private void OnGroupsChanged(Object sender, NotifyCollectionChangedEventArgs e) => this.CollectionChanged?.Invoke(sender, e);

        public Army()
        {
            this.groups.CollectionChanged += this.OnGroupsChanged;
            this.groups.ItemChanged += this.OnUnitCountChanged;
        }

        public Army(ICollection<UnitGroup> groups)
        {
            if (Object.ReferenceEquals(groups, null)) throw new ArgumentNullException(nameof(groups));

            var types = new HashSet<UnitType>();
            var sortedGroups = new List<UnitGroup>(groups.Count);
            foreach (var g in groups)
            {
                if (!types.Add(g.Unit))
                {
                    sortedGroups.Clear();
                    break;
                }
                sortedGroups.Add(g);
            }
            sortedGroups.Sort();
            foreach (var g in sortedGroups) this.groups.Add(g);

            this.groups.CollectionChanged += this.OnGroupsChanged;
            this.groups.ItemChanged += this.OnUnitCountChanged;
        }

        public void CopyTo(Army other)
        {
            if (Object.ReferenceEquals(other, null)) throw new ArgumentNullException(nameof(other));
            other.doHoldNotifications = true;

            other.camp = this.camp;
            other.frenzyBonus = this.frenzyBonus;
            this.skills.CopyTo(other.skills);
            other.traits.Clear();
            foreach (var t in this.traits) other.traits.Add(t);

            other.groups.Clear();
            foreach (var g in this.groups) other.groups.Add(g);

            other.doHoldNotifications = false;
            other.NotifyReset();
        }

        private void OnUnitCountChanged(Object sender, EventArgs e)
        {
            if (this.doHoldNotifications) return;
            var handler = this.PropertyChanged;
            if (Object.ReferenceEquals(handler, null)) return;

            handler(this, new PropertyChangedEventArgs(nameof(this.Types)));
            handler(this, new PropertyChangedEventArgs(nameof(this.CountsByType)));
            handler(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
            handler(this, new PropertyChangedEventArgs(nameof(this.CountGroups)));
            handler(this, new PropertyChangedEventArgs(nameof(this.CountUnits)));
            handler(this, new PropertyChangedEventArgs(nameof(this.Capacity)));
        }

        private void NotifyReset()
        {
            if (this.doHoldNotifications) return;
            var handler = this.PropertyChanged;
            if (Object.ReferenceEquals(handler, null)) return;

            handler(this, new PropertyChangedEventArgs(nameof(this.Camp)));
            handler(this, new PropertyChangedEventArgs(nameof(this.FrenzyBonus)));
            handler(this, new PropertyChangedEventArgs(nameof(this.Skills)));
            handler(this, new PropertyChangedEventArgs(nameof(this.Traits)));

            handler(this, new PropertyChangedEventArgs(nameof(this.Types)));
            handler(this, new PropertyChangedEventArgs(nameof(this.CountsByType)));
            handler(this, new PropertyChangedEventArgs(nameof(this.IsEmpty)));
            handler(this, new PropertyChangedEventArgs(nameof(this.CountGroups)));
            handler(this, new PropertyChangedEventArgs(nameof(this.CountUnits)));
            handler(this, new PropertyChangedEventArgs(nameof(this.Capacity)));
        }

        private Boolean Has(Func<UnitType, Boolean> filter)
        {
            if (Object.ReferenceEquals(filter, null)) filter = u => true;
            foreach (var g in this.groups) if (filter(g.Unit)) return true;
            return false;
        }

        public Boolean Has(UnitFaction faction) => this.Has(u => u.Faction == faction);
        public Boolean Has(UnitCategory category) => this.Has(u => u.Category == category);
        public Boolean Has(BattleTrait trait) => this.traits.Contains(trait);

        public Boolean DoSkipEmptySkills
        {
            get => this.skills.DoSkipDefault;
            set => this.skills.DoSkipDefault = value;
        }

        public Camp Camp
        {
            get => this.camp;
            set
            {
                this.camp = value;
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Camp)));
            }
        }

        public Double FrenzyBonus
        {
            get => this.frenzyBonus;
            set
            {
                if (value < 0.0) value = 0.0;
                this.frenzyBonus = value;
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.FrenzyBonus)));
            }
        }

        public EnumArray<BattleSkill, Int32> Skills => this.skills;

        public List<BattleTrait> Traits => this.traits;
        
        public IEnumerable<UnitType> Types => from g in this.groups select g.Unit;

        public IEnumerable<Int32> CountsByType => from g in this.groups select g.Count;

        public Boolean IsEmpty => this.groups.Count == 0;

        public Int32 CountGroups => this.groups.Count;

        public Int32 CountUnits
        {
            get
            {
                var count = 0;
                foreach (var g in this.groups) count += g.Count;
                return count;
            }
        }

        public Int32 Capacity
        {
            get
            {
                var bonus = 5 * this.skills[BattleSkill.GarrisonAnnex];
                var capacity = 0;
                foreach (var g in this.groups) capacity += (g.Count * g.Unit.Capacity);
                return capacity + bonus;
            }
        }

        public String ToString(Func<UnitType, String> format)
        {
            if (Object.ReferenceEquals(format, null)) format = Army.DefaultFormat;

            if (this.groups.Count == 0) return Army.EmptyArmyString;
            var groupStrings = new List<String>(this.groups.Count);
            foreach (var g in this.groups) groupStrings.Add($"{g.Count}{format(g.Unit)}");
            return String.Join(" ", groupStrings);
        }

        public String ToCompactString() => this.ToString(Army.CompactFormat);

        public override String ToString() => this.ToString(Army.DefaultFormat);

        public override Int32 GetHashCode()
        {
            return this.groups.GetHashCode();
        }
    }
}
