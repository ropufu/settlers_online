using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.UpdaterApp
{
    public class FileInstruction : QuietObject, IInstruction
    {
        private readonly Object syncLock = new Object();
        private String copyFrom = null;
        private String copyTo = null;
        private FileInstruction rollback = null;

        /// <summary>
        /// Path to the (updated) file to copy from.
        /// </summary>
        /// <exception cref="ArgumentNullException"></exception>
        public String CopyFrom
        {
            get => this.copyFrom;
            set
            {
                lock (this.syncLock)
                {
                    if (value.IsNull()) throw new ArgumentNullException(nameof(value));
                    this.copyFrom = value;
                }
            }
        }

        /// <summary>
        /// Path to the (old) file to overwrite.
        /// </summary>
        /// <exception cref="ArgumentNullException"></exception>
        public String CopyTo
        {
            get => this.copyTo;
            set
            {
                lock (this.syncLock)
                {
                    if (value.IsNull()) throw new ArgumentNullException(nameof(value));
                    this.copyTo = value;
                }
            }
        }

        public Boolean ReadFrom(StreamReader reader)
        {
            if (reader.IsNull()) throw new ArgumentNullException(nameof(reader));
            lock (this.syncLock)
            {
                var from = default(String);
                var to = default(String);
                if (!reader.TryReadLine(out from)) return false;
                if (!reader.TryReadLine(out to)) return false;

                this.copyFrom = from;
                this.copyTo = to;
                return true;
            }
        }

        public Boolean WriteTo(StreamWriter writer)
        {
            if (writer.IsNull()) throw new ArgumentNullException(nameof(writer));
            lock (this.syncLock)
            {
                try
                {
                    writer.WriteLine(this.copyFrom);
                    writer.WriteLine(this.copyTo);
                    return true;
                }
                catch (ObjectDisposedException e) { return this.OnError(e); }
                catch (IOException e) { return this.OnError(e); }
            }
        }

        public Boolean TryExecute()
        {
            lock (this.syncLock)
            {
                if (this.copyFrom.IsNull()) return false;
                if (this.copyTo.IsNull()) return false;
                this.rollback = null;

                try
                {
                    // Two scenarios:
                    // 
                    // |  From   |  To      |  To Update  |  Action               |  Rollback                |
                    // |--------------------|-------------|--------------------------------------------------|
                    // |  <...>  |  empty   |  [from]     |  Delete [from]        | Copy [backup] -> [from]  |
                    // |  <...>  |  <...>   |  [to]       |  Copy [from] -> [to]  | Copy [backup] -> [to]    |
                    // |  <...>  |  <...>*  |  [to]       |  Copy [from] -> [to]  | Delete [to]              |
                    // 
                    // * denotes new file.
                    // 
                    var doDelete = String.IsNullOrEmpty(this.copyTo);
                    var isNew = (!doDelete) && (!File.Exists(this.copyTo));
                    var fileToUpdate = doDelete ? this.copyFrom : this.copyTo;

                    var backupPath = default(String);
                    if (!isNew)
                    {
                        // Create a backup copy.
                        backupPath = Path.GetTempFileName();
                        File.Copy(fileToUpdate, backupPath, true);
                    }

                    // Perform action.
                    if (doDelete) File.Delete(this.copyFrom);
                    else File.Copy(this.copyFrom, this.copyTo, true);

                    // Store the rollback path.
                    if (isNew) this.rollback = new FileInstruction() { CopyFrom = fileToUpdate, CopyTo = String.Empty };
                    else this.rollback = new FileInstruction() { CopyFrom = backupPath, CopyTo = fileToUpdate };

                    return true;
                }
                catch (PathTooLongException e) { return this.OnError(e); }
                catch (DirectoryNotFoundException e) { return this.OnError(e); }
                catch (FileNotFoundException e) { return this.OnError(e); }
                catch (IOException e) { return this.OnError(e); }
                catch (ArgumentNullException e) { return this.OnError(e); }
                catch (ArgumentException e) { return this.OnError(e); }
                catch (UnauthorizedAccessException e) { return this.OnError(e); }
                catch (NotSupportedException e) { return this.OnError(e); }
            }
        }

        public Boolean Rollback()
        {
            if (this.rollback.IsNull()) return true;
            if (this.rollback.TryExecute()) return true;
            return this.OnError(this.rollback.LastError);
        }

        public override String ToString() => $"[{this.copyFrom}] -> [{this.copyTo}]";

        public override Int32 GetHashCode()
        {
            lock (this.syncLock)
            {
                if (this.copyFrom.IsNull()) return 0;
                if (this.copyTo.IsNull()) return 0;
                return this.copyFrom.GetHashCode() ^ this.copyTo.GetHashCode();
            }
        }
    }
}
