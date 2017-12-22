using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    using CharTree = PrefixTree<Char, String>;
    
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class AdventureDatabase : PrefixDatabase<Adventure>
    {
        private static readonly AdventureDatabase instance = new AdventureDatabase();

        public static AdventureDatabase Instance => AdventureDatabase.instance;

        public static String BuildKey(Adventure unit) => unit.Name;

        public static String BuildPrimaryName(Adventure unit) => unit.Name.RelaxCase();

        public static IEnumerable<String> Names(Adventure unit) => unit.Keys;

        protected override String OverrideBuildKey(Adventure unit) => AdventureDatabase.BuildKey(unit);

        protected override String OverrideBuildPrimaryName(Adventure unit) => AdventureDatabase.BuildPrimaryName(unit);

        protected override IEnumerable<String> OverrideNames(Adventure unit) => AdventureDatabase.Names(unit);

        private HashSet<String> keys = new HashSet<String>();

        public AdventureDatabase()
        {

        }

        protected override void OnClear()
        {
            this.keys.Clear();
        }

        public IEnumerable<Adventure> Adventures => from pair in this.Database select pair.Value;

        protected override void OnLoading(ref Adventure unit, out Boolean doCancel)
        {
            doCancel = false;
            unit.Trim();
            unit.BuildKeys();
            foreach (var key in unit.Keys)
            {
                if (this.keys.Contains(key))
                {
                    App.Warnings.Push($"Adventure with the same key ({key}) already loaded.");
                    doCancel = true;
                    break;
                }
            }
        }

        protected override void OnLoaded(ref Adventure unit)
        {
            foreach (var key in unit.Keys) this.keys.Add(key);
        }
    }
}
