using Newtonsoft.Json;
using Ropufu.Aftermath;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows;

namespace Ropufu.LeytePond.Bridge
{
    public class ReportList : List<ReportEntry>
    {

    }

    [JsonObject(MemberSerialization.OptIn)]
    public class Report : IEnumerable<ReportEntry>
    {
        [JsonProperty("report", Required = Required.Always)]
        private List<ReportEntry> entries = new List<ReportEntry>();

        public List<ReportEntry> Entries { get => this.entries; }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="path">Path to .cbor file.</param>
        public static Report FromCbor(String path)
        {
            try
            {
                if (!System.IO.File.Exists(path))
                {
                    App.Warnings.Push($"Invalid location for cbor file.");
                    return null;
                }
                using (var stream = new System.IO.FileStream(path, System.IO.FileMode.Open))
                {
                    var cbor = PeterO.Cbor.CBORObject.Read(stream);
                    var json = cbor.ToJSONString();
                    return JsonConvert.DeserializeObject<Report>(json);
                }
            }
            catch (JsonReaderException)
            {
                App.Warnings.Push($"Error while parsing file ({path}).");
            }
            catch (JsonSerializationException)
            {
                App.Warnings.Push($"Error while deserializing file ({path}).");
            }
            catch (PeterO.Cbor.CBORException)
            {
                App.Warnings.Push($"Error while reconstructing cbor file ({path}).");
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
            return null;
        }
        
        public IEnumerator<ReportEntry> GetEnumerator() => this.entries.GetEnumerator();

        IEnumerator IEnumerable.GetEnumerator() => this.GetEnumerator();
    }

    /** Mirrors structural behavior of \c damage.hpp. */
    [JsonObject(MemberSerialization.OptIn)]
    public class ReportEntry
    {
        [JsonProperty("header", Required = Required.Always)]
        private Boolean isHeader = false;
        [JsonProperty("caption", Required = Required.Always)]
        private String caption = String.Empty;
        [JsonProperty("details")]
        private String details = String.Empty;
        [JsonProperty("clipboard text")]
        private String clipboardText = String.Empty;
        [JsonProperty("unit name")]
        private String unitName = null;
        [JsonProperty("lower bound")]
        private Int32? lowerBound = null;
        [JsonProperty("upper bound")]
        private Int32? upperBound = null;
        [JsonProperty("observed values")]
        private List<Int32> values = null;
        [JsonProperty("observed counts")]
        private List<Int32> counts = null;

        private UIElement customUI = null;
        private EmpiricalMeasure<Int32> histogram = null;

        private void Validate<T>(T dummy)
        {
            if (Object.ReferenceEquals(this.caption, null)) throw new ArgumentNullException(nameof(this.caption));
            if (Object.ReferenceEquals(this.details, null)) throw new ArgumentNullException(nameof(this.details));

            var hasValues = !Object.ReferenceEquals(this.values, null);
            var hasCounts = !Object.ReferenceEquals(this.counts, null);
            if (hasValues ^ hasCounts) throw new ArgumentNullException(hasValues ? nameof(this.counts) : nameof(this.values));
            if (hasValues)
            {
                if (this.values.Count != this.counts.Count) throw new ShouldNotHappenException();
            }
        }

        public Boolean IsHeader { get => this.isHeader; set => this.isHeader = value; }
        public String Caption { get => this.caption; set => this.Validate(this.caption = value); }
        public String Details { get => this.details; set => this.Validate(this.details = value); }
        public String ClipboardText { get => this.clipboardText; set => this.clipboardText = value; }
        public String UnitName { get => this.unitName; set => this.unitName = value; }
        public Int32? LowerBound { get => this.lowerBound; set => this.lowerBound = value; }
        public Int32? UpperBound { get => this.upperBound; set => this.upperBound = value; }
        public UIElement CustomUI { get => this.customUI; set => this.customUI = value; }
        public EmpiricalMeasure<Int32> Histogram => this.histogram;

        public Double? Mean => Object.ReferenceEquals(this.histogram, null) ? new Double?() : this.histogram.Mean();
        public Double ChancesOfMin => Object.ReferenceEquals(this.histogram, null) ? 0.0 : (100 * this.histogram[this.histogram.Min]);
        public Double ChancesOfMax => Object.ReferenceEquals(this.histogram, null) ? 0.0 : (100 * this.histogram[this.histogram.Max]);

        public String ImageUri => this.unitName;

        public Boolean IsDesigner
        {
            get => false;
            set
            {
                if (value == false) return;
                //var random = new Random();
                //var count = random.Next(1, 5);
                //this.values = new List<Int32>(count);
                //this.counts = new List<Int32>(count);

                //var from = random.Next(10, 20);
                //for (var i = 0; i < count; ++i)
                //{
                //    this.values.Add(from + i);
                //    this.counts.Add(random.Next(1, 100));
                //}
                this.values = new List<Int32>() { 4, 5, 6, 8 };
                this.counts = new List<Int32>() { 2, 5, 5, 4 };
                this.BuildHistogram();
            }
        }

        public List<Int32> DesignerValues
        {
            get => this.values;
            set => this.values = value;
        }

        public List<Int32> DesignerCounts
        {
            get => this.counts;
            set => this.counts = value;
        }

        public void BuildHistogram()
        {
            this.Validate(false);
            this.histogram = null;
            if (Object.ReferenceEquals(this.values, null)) return;

            var doubleCounts = new Double[this.counts.Count];
            for (var i = 0; i < doubleCounts.Length; i++) doubleCounts[i] = (Double)this.counts[i];
            this.histogram = new EmpiricalMeasure<Int32>(this.values.ToArray(), doubleCounts);
        }

        public Boolean IsDegenerate
        {
            get
            {
                if (Object.ReferenceEquals(this.values, null)) return false;
                if (!this.lowerBound.HasValue || !this.upperBound.HasValue) return false;
                return this.lowerBound.Value == this.upperBound.Value;
            }
        }

        public Boolean HasHistogram => !Object.ReferenceEquals(this.values, null);

        public Boolean HasNonDegenerateHistogram
        {
            get
            {
                if (Object.ReferenceEquals(this.values, null)) return false;
                if (this.values.Count == 0) return false;
                var isFirst = true;
                var min = 0;
                var max = 0;
                foreach (var x in this.values)
                {
                    if (isFirst) min = max = x;
                    if (x > max) max = x;
                    if (x < min) min = x;
                    isFirst = false;
                }
                return min != max;
            }
        }

        public Visibility HistogramVisibility => Object.ReferenceEquals(this.values, null) ? Visibility.Hidden : Visibility.Visible;
    }
}
