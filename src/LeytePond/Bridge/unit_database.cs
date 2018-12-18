using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class UnitDatabase : NameDatabase<UnitType, Int32>
    {
        protected override int OnBuildKey(UnitType unit) => unit.Id;

        protected override HashSet<String> OnBuildNames(UnitType unit) => new HashSet<String>(unit.Names);

        protected override void OnRelaxed(UnitType unit, HashSet<String> relaxedNames, ref HashSet<String> strictNames)
        {
            foreach (var code in unit.Codenames) strictNames.Add(code);
        }

        public UnitDatabase()
        {
        }

        public IEnumerable<UnitType> Generals => from pair in this.Data where pair.Value.Is(UnitFaction.General) select pair.Value;

        protected override void OnLoading(UnitType unit, ref Boolean doCancel)
        {
            unit.Trim();
        }
    }
}
