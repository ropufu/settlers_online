using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    using CharTree = PrefixTree<Char, String>;
    
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class CampDatabase : PrefixDatabase<Camp>
    {
        private static readonly CampDatabase instance = new CampDatabase();

        public static CampDatabase Instance => CampDatabase.instance;

        public static String BuildKey(Camp unit) => unit.FirstName.RelaxCase();

        public static String BuildPrimaryName(Camp unit) => unit.FirstName.RelaxCase();

        public static IEnumerable<String> Names(Camp unit) => unit.Names;

        protected override String OverrideBuildKey(Camp unit) => CampDatabase.BuildKey(unit);

        protected override String OverrideBuildPrimaryName(Camp unit) => CampDatabase.BuildPrimaryName(unit);

        protected override IEnumerable<String> OverrideNames(Camp unit) => CampDatabase.Names(unit);

        public CampDatabase()
        {

        }

        protected override void OnClear()
        {
        }

        public IEnumerable<Camp> Camps => from pair in this.Database select pair.Value;

        protected override void OnLoading(ref Camp unit, out Boolean doCancel)
        {
            doCancel = false;
        }

        protected override void OnLoaded(ref Camp unit)
        {
        }
    }
}
