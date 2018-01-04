using System;
using System.ComponentModel;

namespace Ropufu.LeytePond.Bridge
{
    /** Minimal behavior of \c unit_group.hpp with extra bells and whistles for GUI. */
    public class UnitGroup : IComparable<UnitGroup>, INotifyPropertyChanged
    {
        private UnitType unit = new UnitType();
        private Int32 count = 0;
        private Boolean isCoupled = false;

        public event PropertyChangedEventHandler PropertyChanged;

        private void Validate<T>(T dummy)
        {
            if (this.count < 0) throw new ArgumentOutOfRangeException(nameof(this.count));
            if (Object.ReferenceEquals(this.unit, null)) throw new ArgumentOutOfRangeException(nameof(this.unit));
        }

        public UnitGroup()
        {
        }

        public UnitGroup(UnitType unit, Int32 count)
        {
            this.unit = unit;
            this.count = count;
            this.Validate(true);
        }

        public Int32 CompareTo(UnitGroup other) => this.unit.CompareTo(other.unit);

        public UnitType Unit
        {
            get => this.unit;
            set
            {
                this.Validate(this.unit = value);
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Unit)));
            }
        }

        public Int32 Count
        {
            get => this.count;
            set
            {
                this.Validate(this.count = value);
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Count)));
            }
        }

        public Boolean IsCoupled
        {
            get => this.isCoupled;
            set
            {
                this.isCoupled = value;
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsCoupled)));
            }
        }
    }
}
