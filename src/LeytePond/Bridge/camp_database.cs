using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class CampDatabase : NameDatabase<Camp, String>
    {
        protected override String OnBuildKey(Camp unit) => unit.FirstName.RelaxCase();

        protected override HashSet<String> OnBuildNames(Camp unit) => new HashSet<String>(unit.Names);

        public CampDatabase()
        {

        }

        protected override void OnLoading(Camp unit, ref Boolean doCancel)
        {
            unit.Trim();
        }
    }
}
