using System;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c army_parser.hpp. */
    public class ArmyParser
    {
        private String value = String.Empty;
        private Boolean isValid = false;
        private List<KeyValuePair<String, Int32>> armyBlueprint = new List<KeyValuePair<String, Int32>>();

        private void ParseBlueprint(String value)
        {
            var zero = (Int32)('0');
            this.isValid = false;

            var count = 0;
            var key = String.Empty;

            var position = 0;
            var firstIndex = 0;
            var lastIndex = 0;
            while (position < value.Length)
            {
                // Scan mode: we are building a number.
                count = 0;
                firstIndex = position;
                for (; position < value.Length; ++position)
                {
                    var c = value[position];
                    if (c < '0' || c > '9') break; // Non-digit encountered!

                    count *= 10;
                    count += ((Int32)(c) - zero);
                }
                if (firstIndex == position) return; // The first non-whitespace character was not a digit: invalid format.

                for (; position < value.Length; ++position) if (value[position] != ' ') break; // Skip the leading whitespaces (at most one---cf. assumptions).

                // Scan mode: we are now building a name.
                firstIndex = position;
                lastIndex = 0;
                for (; position < value.Length; ++position)
                {
                    var c = value[position];
                    if (c >= '0' && c <= '9') break; // Digit encountered!
                    if (c != ' ') lastIndex = position; // Skip the trailing whitespaces.
                }
                if (lastIndex == 0) return; // The first non-whitespace character was a digit (not a letter!): invalid format.
                key = value.Substring(firstIndex, lastIndex - firstIndex + 1);
                this.armyBlueprint.Add(new KeyValuePair<String, Int32>(key, count));
            }
            this.isValid = true;
        }
        
        public ArmyParser(String value)
        {
            this.value = value.DeepTrim();
            this.ParseBlueprint(this.value);
            if (!this.isValid) this.armyBlueprint.Clear();
        }

        public String Value => this.value;
        public Boolean IsGood => this.isValid;
        public Int32 Count => this.armyBlueprint.Count;

        public Boolean TryBuild(ref Army a, Func<UnitType, Boolean> filter = null)
        {
            if (Object.ReferenceEquals(filter, null)) filter = u => true;
            if (!this.isValid) return false;

            var groups = new List<UnitGroup>(this.armyBlueprint.Count);
            foreach (var pair in this.armyBlueprint)
            {
                var u = new UnitType();
                if (!UnitDatabase.Instance.TryFind(pair.Key, ref u, filter)) return false;

                if (pair.Value == 0) continue; // Skip empty groups.
                groups.Add(new UnitGroup(u, pair.Value));
            }
            a = new Army(groups);
            return true;
        }

        public Army Build(Logger warnings, Boolean doCheckGenerals = false, Boolean doCoerceFactions = false, Boolean isStrct = false)
        {
            var army = new Army();
            if (!this.isValid)
            {
                warnings.Push($"Parsing army failed.");
                return army;
            }

            if (!this.TryBuild(ref army, null))
            {
                warnings.Push($"Reconstructing army from database failed.");
                return army;
            }

            if (army.IsEmpty) return army; // There is nothing more to be done if the army is empty.

            // Check for missing generals.
            if (doCheckGenerals)
            {
                if (!army.Has(UnitFaction.General)) warnings.Push($"Army does not have any generals.");
            }

            // Check for multiple factions.
            if (doCoerceFactions)
            {
                var factions = new HashSet<UnitFaction>();
                foreach (var g in army) factions.Add(g.Unit.Faction);
                factions.Remove(UnitFaction.General); // Generals do not count.

                var countOptions = 0;
                if (factions.Count > 1)
                {
                    warnings.Push("There is more than one faction in the army.");
                    var a = new Army();
                    foreach (var f in factions)
                    {
                        // Try to re-build this army assuming only fraction <f> is allowed (or generals).
                        if (this.TryBuild(ref a, u => u.Is(f) || u.Is(UnitFaction.General)))
                        {
                            ++countOptions;
                            warnings.Push($"Did you mean: {a.ToCompactString()}?");
                        }
                    }
                    // If there is only one alternative with single faction, take it!
                    if (countOptions == 1 && !isStrct)
                    {
                        warnings.Push("Assuming yes.");
                        army = a;
                    }
                }
            }
            return army;
        }

        public override String ToString()
        {
            if (!this.isValid || this.armyBlueprint.Count == 0) return String.Empty;
            var groupStrings = new List<String>(this.armyBlueprint.Count);
            foreach (var g in this.armyBlueprint) groupStrings.Add($"{g.Value} {g.Key}");
            return String.Join(" ", groupStrings);
        }
        
        public override Int32 GetHashCode() => this.armyBlueprint.GetHashCode();
    }
}
