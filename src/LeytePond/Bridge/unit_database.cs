using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    using CharTree = PrefixTree<Char, String>;
    
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class UnitDatabase : PrefixDatabase<UnitType>
    {
        private static readonly UnitDatabase instance = new UnitDatabase();

        public static UnitDatabase Instance => UnitDatabase.instance;

        public static String BuildKey(UnitType unit)
        {
            var key = unit.FirstName;
            foreach (var name in unit.Names) if (name.Length < key.Length) key = name;
            return key;
        }

        public static String BuildPrimaryName(UnitType unit) => unit.FirstName.RelaxCase();

        public static IEnumerable<String> Names(UnitType unit) => unit.Names;

        protected override String OverrideBuildKey(UnitType unit) => UnitDatabase.BuildKey(unit);

        protected override String OverrideBuildPrimaryName(UnitType unit) => UnitDatabase.BuildPrimaryName(unit);

        protected override IEnumerable<String> OverrideNames(UnitType unit) => UnitDatabase.Names(unit);

        private HashSet<Int32> ids = new HashSet<Int32>();

        public UnitDatabase()
        {

        }

        protected override void OnClear()
        {
            this.ids.Clear();
        }

        public IEnumerable<UnitType> Generals => from pair in this.Database where pair.Value.Is(UnitFaction.General) select pair.Value;

        protected override void OnLoading(ref UnitType unit, out Boolean doCancel)
        {
            doCancel = false;
            unit.Trim();
            if (this.ids.Contains(unit.Id))
            {
                App.Warnings.Push($"Unit with the same id ({unit.Id}) already loaded.");
                doCancel = true;
            }
        }

        protected override void OnLoaded(ref UnitType unit)
        {
            this.ids.Add(unit.Id);
        }
    }
}
