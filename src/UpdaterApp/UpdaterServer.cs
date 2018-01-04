using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;

namespace Ropufu.UpdaterApp
{
    public class UpdaterServer : QuietObject
    {
        private readonly Object syncLock = new Object();

        public Boolean Run(String clientExecutablePath, ICollection<FileInstruction> instructions)
        {
            lock (this.syncLock)
            {
                if (clientExecutablePath.IsNull()) return false;
                if (instructions.IsNull()) return false;

                var client = new Process();
                client.StartInfo.FileName = clientExecutablePath;
                client.StartInfo.UseShellExecute = false;

                try
                {
                    using (var server = new AnonymousPipeServerStream(PipeDirection.Out, HandleInheritability.Inheritable))
                    {
                        server.ReadMode = PipeTransmissionMode.Byte;
                        var pipeId = server.GetClientHandleAsString();

                        // Pass the client process a handle to the server.
                        client.StartInfo.Arguments = pipeId;
                        client.Start();

                        server.DisposeLocalCopyOfClientHandle();

                        try
                        {
                            // Read user input and send that to the client process.
                            using (var writer = new StreamWriter(server))
                            {
                                writer.AutoFlush = true;
                                // Send the sync message and wait for client to receive it.
                                writer.WriteLine(App.SyncMessage);
                                server.WaitForPipeDrain();

                                // Send the current process id.
                                writer.WriteLine(Process.GetCurrentProcess().Id);
                                server.WaitForPipeDrain();

                                // ~~ Information exchange. Send the current process information to the client process. ~~

                                writer.WriteLine(instructions.Count); // First: number of file instructions.
                                server.WaitForPipeDrain();
                                foreach (var item in instructions) item.WriteTo(writer);
                                server.WaitForPipeDrain();
                            }
                        }
                        // Catch the exceptions raised if the pipe is broken or disconnected.
                        catch (IOException e) { return this.OnError(e); }
                    }
                    return true;
                }
                catch (Win32Exception e) { return this.OnError(e); }
                catch (ObjectDisposedException e) { return this.OnError(e); }
                catch (InvalidOperationException e) { return this.OnError(e); }
                catch (ArgumentOutOfRangeException e) { return this.OnError(e); }
                catch (ArgumentException e) { return this.OnError(e); }
                catch (NotSupportedException e) { return this.OnError(e); }
            }
        }
    }
}
