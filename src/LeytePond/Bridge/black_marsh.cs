using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.LeytePond.Bridge
{
    /** Interacts with command line application \c black_marsh. */
    class BlackMarsh
    {
        private static readonly BlackMarsh instance = new BlackMarsh();

        public static BlackMarsh Instance => BlackMarsh.instance;

        private BlackMarsh()
        {

        }

        private String processPath;
        private String cborPath;
        private Boolean isRunning = false;
        private Boolean isLog = false;
        private StringBuilder builder = new StringBuilder();

        public Boolean IsRunning => this.isRunning;
        public String ProcessPath { get => this.processPath; set => this.processPath = value; }
        public String CborPath { get => this.cborPath; set => this.cborPath = value; }
        public String Output => this.builder.ToString();

        public void Execute(String leftArmy, String rightWaves, BattleWeather weather, Boolean isLog = false)
        {
            if (this.isRunning) return;
            this.isLog = isLog;

            this.isRunning = true;
            this.builder = new StringBuilder();
            var keys = isLog ? "-l" : "-r";
            using (var proc = new Process())
            {
                proc.StartInfo.FileName = this.processPath;
                proc.StartInfo.Arguments = $"\"{leftArmy}\" \"{rightWaves}\" -w \"{weather.ToReadable().ToLowerInvariant()}\" {keys}";
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.RedirectStandardError = true;
                proc.StartInfo.CreateNoWindow = true;

                proc.EnableRaisingEvents = true;

                proc.ErrorDataReceived += this.OnDataReceived;
                proc.OutputDataReceived += this.OnDataReceived;
                proc.Exited += this.ExitedHandler;

                proc.Start();

                proc.BeginErrorReadLine();
                proc.BeginOutputReadLine();
                proc.WaitForExit();
            }
        }

        private void ExitedHandler(Object sender, EventArgs e)
        {
            this.isRunning = false;
        }

        public Report Report
        {
            get
            {
                if (this.isRunning) return null;

                var report = this.isLog ? new Report() : Report.FromCbor(this.cborPath);
                if (Object.ReferenceEquals(report, null))
                {
                    report = new Report();
                    App.Warnings.Push($"Failed to read simulation .cbor output.");
                }

                var lines = this.builder.ToString().Split(new String[] { System.Environment.NewLine }, StringSplitOptions.RemoveEmptyEntries);
                foreach (var line in lines) report.Entries.Add(new ReportEntry { IsHeader = true, Caption = line });
                return report;
            }
        }

        private void OnDataReceived(Object sender, DataReceivedEventArgs e)
        {
            this.builder.AppendLine(e.Data);
        }
    }
}
