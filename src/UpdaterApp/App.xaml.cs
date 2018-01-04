using System;
using System.Collections.Generic;
using System.Windows;

namespace Ropufu.UpdaterApp
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public const String SyncMessage = "OEIS-A011541";

        public static new App Current => (App)Application.Current;

        public event EventHandler<InstructionEventArgs> InstructionExecuting;
        public event EventHandler<InstructionEventArgs> InstructionCompleted;
        public event EventHandler<InstructionEventArgs> InstructionRollback;

        private Boolean isGood = false;

        public Boolean IsGood => this.isGood;

        public Int32 CountInstructions
        {
            get
            {
                if (this.client.IsNull()) return 0;
                return this.client.Instructions?.Count ?? 0;
            }
        }

        private UpdaterClient client = null;

        private Boolean InitializeUpdater(String[] args)
        {
            if (args.IsNull()) return false;
            if (args.Length == 0) return false;
            if (args[0].IsNull()) return false;

            this.client = new UpdaterClient(args[0]);
            return this.client.Run();
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            this.isGood = this.InitializeUpdater(e.Args);
            if (!this.isGood)
            {
                this.Shutdown();
            }
        }

        public Boolean Execute()
        {
            if (!this.isGood) return false;
            if (this.client.IsNull()) return false;
            if (this.client.Instructions.IsNull()) return false;

            this.client.WaitForServerShutdown();
            var instructions = this.client.Instructions;

            var onExecuting = this.InstructionExecuting;
            var onCompleted = this.InstructionCompleted;
            var completed = new Stack<IInstruction>();
            foreach (var instruction in instructions)
            {
                onExecuting?.Invoke(this, new InstructionEventArgs(instruction));

                var hasCompleted = instruction.TryExecute();
                if (!hasCompleted) break;

                completed.Push(instruction);
                onCompleted?.Invoke(this, new InstructionEventArgs(instruction));
            }
            // Rollback.
            if (completed.Count != instructions.Count)
            {
                var onRollback = this.InstructionRollback;
                while (completed.Count > 0)
                {
                    var instruction = completed.Pop();
                    instruction.Rollback();
                    onRollback?.Invoke(this, new InstructionEventArgs(instruction));
                }
                return false;
            }
            return true;
        }

        public void RestartServer() => this.client?.RestartServer();
    }
}
