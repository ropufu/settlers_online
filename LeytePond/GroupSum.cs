using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;

namespace Ropufu.LeytePond
{
    public class GroupSum : INotifyPropertyChanged
    {
        private GroupCountUpDown master;
        private HashSet<GroupCountUpDown> children = new HashSet<GroupCountUpDown>();
        private Int32 countCoupled = 0;
        private Boolean isFixedSumLocked = false;

        public event PropertyChangedEventHandler PropertyChanged;
        
        public Boolean CanCouple => this.countCoupled < 2;

        private static Int32 EnsureValue(GroupCountUpDown item)
        {
            var value = item.Value;
            if (!value.HasValue) throw new NotSupportedException(); // <Null> values not allowed.
            return value.Value;
        }

        public void AddChild(GroupCountUpDown item)
        {
            if (Object.ReferenceEquals(item, null)) throw new ArgumentNullException(nameof(item));
            
            this.children.Add(item);
            this.InvalidateConstraint();
        }

        public Boolean RemoveChild(GroupCountUpDown item)
        {
            if (Object.ReferenceEquals(item, null)) throw new ArgumentNullException(nameof(item));

            if (!this.children.Remove(item)) return false;
            this.InvalidateConstraint();
            return true;
        }

        public void ClearChildren()
        {
            this.children.Clear();
            this.InvalidateConstraint();
        }
        
        public Int32 CurrentSum => this.children.Sum(item => item.Value.Value);

        public IList<GroupCountUpDown> Children => this.children.ToList().AsReadOnly();

        public GroupCountUpDown Master
        {
            get => this.master;
            set
            {
                this.master = value;
                this.InvalidateConstraint();
            }
        }

        /// <summary>
        /// Updates the <see cref="GroupCountUpDown.MaximumProperty"/> on children, and <see cref="GroupCountUpDown.MinimumProperty"/> on master.
        /// </summary>
        public void InvalidateConstraint()
        {
            var noMaster = Object.ReferenceEquals(this.master, null);
            var noConstraint = noMaster || (!this.master.Value.HasValue);

            // First pass: build up statistics.
            var sum = 0;
            var fixedSum = 0;
            this.countCoupled = 0;
            foreach (var item in this.children)
            {
                var value = GroupSum.EnsureValue(item);
                sum += value;
                if (item.IsCoupled)
                {
                    if (!this.CanCouple) throw new NotSupportedException(); // Coupling not allowed.
                    this.countCoupled++;
                    fixedSum += value;
                }
            }
            var isSingular = this.countCoupled == 1;

            // Second pass: update properties.
            if (!noMaster) this.master.Minimum = sum;
            if (noConstraint)
            {
                foreach (var item in this.children)
                {
                    var isCoupled = item.IsCoupled;
                    item.Minimum = (isSingular && isCoupled) ? fixedSum : 0;
                    item.Maximum = isCoupled ? fixedSum : Int32.MaxValue;
                }
            }
            else
            {
                var localMaximum = this.master.Value.Value - sum;
                foreach (var item in this.children)
                {
                    var isCoupled = item.IsCoupled;
                    item.Minimum = (isSingular && isCoupled) ? fixedSum : 0;
                    item.Maximum = isCoupled ? fixedSum : (localMaximum + item.Value.Value);
                }
            }

            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.CurrentSum)));
        }

        public void OnValueChanged(GroupCountUpDown sender, Int32? oldValue, Int32? newValue)
        {
            if (Object.ReferenceEquals(sender, null)) throw new ArgumentNullException(nameof(sender));
            if (!oldValue.HasValue || !newValue.HasValue) throw new NotSupportedException();
            Debug.Assert(this.children.Contains(sender), "Sender not recognized.");

            var delta = newValue.Value - oldValue.Value;
            if (sender.IsCoupled)
            {
                if (this.isFixedSumLocked) return; // If this item is coupled, the value change will be handled in the following block.
                this.isFixedSumLocked = true; // Make sure the other coupled items don't call the block.

                foreach (var item in this.children)
                {
                    if (Object.ReferenceEquals(item, sender)) continue;
                    if (!item.IsCoupled) continue; // Skip uncoupled items.

                    var gap = item.Value.Value; // <gap> = <old value>.
                    item.Value -= delta;
                    gap -= item.Value.Value; // <gap> = <old value> - <new value>.
                    delta -= gap;

                    if (delta == 0) break;
                }
                Debug.Assert(delta == 0, "Maximum not set properly on coupled items.");
                this.isFixedSumLocked = false; // All coupled items have been processed.
            }
            else
            {
                var noMaster = Object.ReferenceEquals(this.master, null);
                var noConstraint = noMaster || (!this.master.Value.HasValue);

                foreach (var item in this.children)
                {
                    if (Object.ReferenceEquals(item, sender)) continue;
                    if (item.IsCoupled) continue; // Skip coupled items.

                    if (noConstraint) item.Maximum = Int32.MaxValue;
                    else item.Maximum -= delta;
                }
                if (!noMaster) this.master.Minimum += delta;
            }

            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.CurrentSum)));
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.Children)));
        }
    }
}
