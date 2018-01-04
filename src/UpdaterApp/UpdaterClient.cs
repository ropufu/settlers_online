using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;

namespace Ropufu.UpdaterApp
{
    public class UpdaterClient : QuietObject
    {
        private readonly Object syncLock = new Object();
        private String pipeHandleString = null;
        private Process serverProcess = null;
        private String serverPath = null;
        private TimeSpan timeout = System.Threading.Timeout.InfiniteTimeSpan;
        private List<IInstruction> instructions = new List<IInstruction>();

        public IList<IInstruction> Instructions => this.instructions.AsReadOnly();

        public UpdaterClient(String pipeHandleString)
        {
            if (pipeHandleString.IsNull()) throw new ArgumentNullException(nameof(pipeHandleString));

            this.pipeHandleString = pipeHandleString;
        }

        /// <summary>
        /// How long to wait for the server process to finish.
        /// </summary>
        public TimeSpan Timeout
        {
            get => this.timeout;
            set
            {
                lock (this.syncLock)
                {
                    this.timeout = value;
                }
            }
        }

        public Boolean WaitForServerShutdown()
        {
            lock (this.syncLock)
            {
                if (this.serverProcess.IsNull()) return false;

                try
                {
                    if (!this.serverProcess.WaitForExit(this.timeout.Milliseconds)) return false;

                    return true;
                }
                catch (Win32Exception e) { return this.OnError(e); }
                catch (ArgumentOutOfRangeException e) { return this.OnError(e); }
                catch (SystemException e) { return this.OnError(e); }
            }
        }

        public Boolean RestartServer()
        {
            lock (this.syncLock)
            {
                if (this.serverProcess.IsNull()) return false;

                try
                {
                    if (!this.serverProcess.HasExited) return false;
                    var renewed = new Process();
                    renewed.StartInfo.FileName = this.serverPath;
                    renewed.StartInfo.UseShellExecute = false;
                    renewed.Start();

                    this.serverProcess = renewed;
                    return true;
                }
                catch (Win32Exception e) { return this.OnError(e); }
                catch (PlatformNotSupportedException e) { return this.OnError(e); }
                catch (ObjectDisposedException e) { return this.OnError(e); }
                catch (InvalidOperationException e) { return this.OnError(e); }
                catch (NotSupportedException e) { return this.OnError(e); }
                catch (ArgumentNullException e) { return this.OnError(e); }
            }
        }

        public Boolean Run()
        {
            lock (this.syncLock)
            {
                this.instructions.Clear();
                this.serverProcess = null;

                try
                {
                    using (var client = new AnonymousPipeClientStream(PipeDirection.In, this.pipeHandleString))
                    {
                        client.ReadMode = PipeTransmissionMode.Byte;
                        using (var reader = new StreamReader(client))
                        {
                            var temp = default(String);
                            if (!reader.TryReadLine(out temp)) return false;

                            // Wait for the sync message from the server.
                            while (!temp.StartsWith(App.SyncMessage))
                            {
                                if (!reader.TryReadLine(out temp)) return false;
                            }

                            // Get the server process id.
                            var serverProcessId = 0;
                            if (!reader.TryReadInt32(ref serverProcessId)) return false;
                            try
                            {
                                this.serverProcess = Process.GetProcessById(serverProcessId);
                                this.serverPath = this.serverProcess.MainModule.FileName;
                            }
                            catch (Win32Exception e) { return this.OnError(e); }
                            catch (PlatformNotSupportedException e) { return this.OnError(e); }
                            catch (NotSupportedException e) { return this.OnError(e); }
                            catch (ArgumentException e) { return this.OnError(e); }
                            catch (InvalidOperationException e) { return this.OnError(e); }

                            // ~~ Information exchange. ~~

                            var countInstructions = 0;
                            if (!reader.TryReadInt32(ref countInstructions)) return false; // First: number of file instructions.
                            for (var i = 0; i < countInstructions; ++i)
                            {
                                var instruction = new FileInstruction();
                                if (!instruction.ReadFrom(reader)) return false;
                                this.instructions.Add(instruction);
                            }
                        }
                    }
                    return true;
                }
                catch (ArgumentNullException e) { return this.OnError(e); }
                catch (ArgumentException e) { return this.OnError(e); }
                catch (NotSupportedException e) { return this.OnError(e); }
            }
        }
    }
}
