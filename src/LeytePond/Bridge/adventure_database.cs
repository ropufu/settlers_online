using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c unit_database.hpp. */
    public class AdventureDatabase : NameDatabase<Adventure, String>
    {
        protected override String OnBuildKey(Adventure unit) => unit.Name;

        protected override HashSet<String> OnBuildNames(Adventure unit) => new HashSet<String>(unit.Keys);

        private HashSet<String> keys = new HashSet<String>();

        public AdventureDatabase()
        {

        }

        protected override void OnClear()
        {
            this.keys.Clear();
        }

        protected override void OnLoading(Adventure unit, ref Boolean doCancel)
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

        protected override void OnLoaded(Adventure unit)
        {
            foreach (var key in unit.Keys) this.keys.Add(key);
        }
    }
}
