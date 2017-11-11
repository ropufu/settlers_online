using System;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.ComponentModel;

namespace Ropufu.LeytePond.Bridge
{
    /** Mirrors structural behavior of \c config.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    class Config: INotifyPropertyChanged
    {
        private static Config instance = new Config();
        public static Config Instance => Config.instance;

        private Boolean isGood = false;
        //private Boolean hasChanged = false;
        private String fileName = String.Empty;

        [JsonProperty("maps folder")]
        private String mapsPath = "./maps/";
        [JsonProperty("faces folder")]
        private String facesPath = "./faces/";
        [JsonProperty("skills folder")]
        private String skillsPath = "./skills/";
        [JsonProperty("simulations")]
        private Int32 countCombatSims = 10000;
        [JsonProperty("destructions per sim")]
        private Int32 countDestructSimsPerCombat = 10;
        [JsonProperty("threads")]
        private Int32 countThreads = 1;
        [JsonProperty("left")]
        private ArmyDecorator left = new ArmyDecorator();
        [JsonProperty("right")]
        private ArmyDecorator right = new ArmyDecorator();

        public event PropertyChangedEventHandler PropertyChanged;

        public String MapsPath
        {
            get => this.mapsPath;
            set
            {
                this.mapsPath = value;
                this.Write();
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.MapsPath)));
            }
        }

        public String FacesPath
        {
            get => this.facesPath;
            set
            {
                this.facesPath = value;
                this.Write();
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.FacesPath)));
            }
        }

        public String SkillsPath
        {
            get => this.skillsPath;
            set
            {
                this.skillsPath = value;
                this.Write();
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.SkillsPath)));
            }
        }

        public Int32 CountCombatSims
        {
            get => this.countCombatSims;
            set
            {
                this.countCombatSims = value;
                this.Write();
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.CountCombatSims)));
            }
        }

        public Int32 CountDestructSimsPerCombat
        {
            get => this.countDestructSimsPerCombat;
            set
            {
                this.countDestructSimsPerCombat = value;
                this.Write();
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.CountDestructSimsPerCombat)));
            }
        }

        public Int32 CountThreads
        {
            get => this.countThreads;
            set
            {
                this.countThreads = value;
                this.Write();
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.CountThreads)));
            }
        }

        public ArmyDecorator Left { get => this.left; }
        public ArmyDecorator Right { get => this.right; }

        public Boolean IsGood => this.isGood;

        private void Write()
        {
            if (!System.IO.File.Exists(this.fileName)) return;

            try
            {
                var json = JsonConvert.SerializeObject(this, Formatting.Indented);
                System.IO.File.WriteAllText(this.fileName, json);
            }
            catch (JsonWriterException)
            {
                App.Warnings.Push($"Error while serializing config file.");
            }
            catch (JsonSerializationException)
            {
                App.Warnings.Push($"Error while serializing config file.");
            }
            catch (System.IO.IOException)
            {
                App.Warnings.Push($"Error writing to file ({this.fileName}).");
            }
            catch (System.Security.SecurityException)
            {
                App.Warnings.Push($"Security error writing to file ({this.fileName}).");
            }
            catch (UnauthorizedAccessException)
            {
                App.Warnings.Push($"Authorization error writing to file ({this.fileName}).");
            }
        }

        public static void Read(String path = "./black_marsh.config")
        {
            try
            {
                if (!System.IO.File.Exists(path))
                {
                    App.Warnings.Push($"Invalid location for config file.");
                    return;
                }
                var json = System.IO.File.ReadAllText(path);
                Config.instance.fileName = path;

                var config = JsonConvert.DeserializeObject<Config>(json);
                config.isGood = true;
                config.fileName = path;
                Config.instance = config;
            }
            catch (JsonReaderException)
            {
                App.Warnings.Push($"Error while parsing file ({path}).");
            }
            catch (JsonSerializationException)
            {
                App.Warnings.Push($"Error while deserializing file ({path}).");
            }
            catch (System.IO.IOException)
            {
                App.Warnings.Push($"Error reading file ({path}).");
            }
            catch (System.Security.SecurityException)
            {
                App.Warnings.Push($"Security error reading file ({path}).");
            }
            catch (UnauthorizedAccessException)
            {
                App.Warnings.Push($"Authorization error reading file ({path}).");
            }
        }
    }
}
