using Ropufu.UpdaterApp;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Ropufu.LeytePond
{
    class UpdateManager
    {
        private Window owner = null;

        /// <exception cref="ArgumentNullException"></exception>
        public UpdateManager(Window owner)
        {
            if (owner.IsNull()) throw new ArgumentNullException(nameof(owner));

            this.owner = owner;
        }

        private void OnUpToDate()
        {
            MessageBox.Show(this.owner, "All files are up to date.", "No Updates Available", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        /// <exception cref="ArgumentNullException"></exception>
        private void OnFailed(String message)
        {
            if (message.IsNull()) throw new ArgumentNullException(nameof(message));

            MessageBox.Show(this.owner, message, "Update Failed", MessageBoxButton.OK, MessageBoxImage.Error);
        }

        /// <exception cref="ArgumentNullException"></exception>
        private Boolean OnUpdatesAvailable(String message)
        {
            if (message.IsNull()) throw new ArgumentNullException(nameof(message));

            var result = MessageBox.Show(this.owner, message, "Updates Available", MessageBoxButton.OKCancel, MessageBoxImage.Question);
            return (result == MessageBoxResult.OK);
        }

        public void CheckForUpdates(Boolean isAutomatic = false)
        {
            var binaries = new List<GitHubFileInfo>();
            try { binaries = this.ListBinaries(); } catch (NotSupportedException) { }

            //           Bin     :  No Bin      
            //        --------------------------
            // Auto   |  ext all :  skip misc  |
            // Manual |  ext all :  local misc |
            //        --------------------------

            var isUpToDate = binaries.Count == 0;
            if (isAutomatic && isUpToDate) return; // Do nothing if it was an automatic call for updates and none were found.

            var misc = this.ListMisc();
            if (isUpToDate) this.LaunchLocal(misc); // There were no updates to binaries, so do a soft update.
            else
            {
                binaries.AddRange(misc); // Add the misc updates to the binary list.
                this.LaunchUpdaterApp(binaries); // Then launch the updater app.
            }
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="OperationFailedException"></exception>
        private List<FileInstruction> MakeLocalCopies(List<GitHubFileInfo> updates)
        {
            if (updates.IsNull()) throw new ArgumentNullException(nameof(updates));

            var instructions = new List<FileInstruction>(updates.Count);
            foreach (var item in updates)
            {
                try
                {
                    var action = item.UpdateAction;
                    var localPath = item.LocalPath;
                    if (action == NotifyCollectionChangedAction.Remove)
                    {
                        instructions.Add(new FileInstruction() { CopyFrom = localPath, CopyTo = String.Empty }); // Mark file for deletion.
                    }
                    else
                    {
                        var temp = Path.GetTempFileName();
                        item.MarkForUpdate(temp, action);
                        if (!item.Update()) throw new OperationFailedException(); // Download the file to a temporary location.
                        instructions.Add(new FileInstruction() { CopyFrom = temp, CopyTo = localPath });
                    }
                }
                catch (NotSupportedException e) { throw new OperationFailedException(e); }
                catch (IOException e) { throw new OperationFailedException(e); }
            }
            return instructions;
        }

        /// <summary>
        /// Installs the updates by calling the updater app and shutting down this application.
        /// </summary>
        /// <exception cref="ArgumentNullException"></exception>
        private void LaunchUpdaterApp(List<GitHubFileInfo> updates)
        {
            if (updates.IsNull()) throw new ArgumentNullException(nameof(updates));
            if (updates.Count == 0)
            {
                this.OnUpToDate();
                return;
            }

            if (!this.OnUpdatesAvailable($"{updates.Count} updates found.{Environment.NewLine}Proceed with download?")) return;

            try
            {
                var instructions = this.MakeLocalCopies(updates); // OperationFailedException.
                var server = new UpdaterServer();
                if (!server.Run("UpdaterApp.exe", instructions))
                {
                    this.OnFailed($"Updater error: {server.LastErrorMessage}.");
                    return;
                }
                App.Current.Shutdown();
            }
            catch (OperationFailedException e)
            {
                var message = e.InnerException.IsNull() ? e.Message : e.InnerException.Message;
                this.OnFailed($"Updater error: {message}.");
            }
        }

        /// <summary>
        /// Installs the updates without quitting the application.
        /// </summary>
        /// <exception cref="ArgumentNullException"></exception>
        private void LaunchLocal(List<GitHubFileInfo> updates)
        {
            if (updates.IsNull()) throw new ArgumentNullException(nameof(updates));

            try
            {
                if (updates.Count == 0) this.OnUpToDate();
                else
                {
                    var builder = new StringBuilder();
                    builder.AppendLine("The following updates are available:"); // ArgumentOutOfRangeException.
                    foreach (var item in updates)
                    {
                        switch (item.UpdateAction)
                        {
                            case NotifyCollectionChangedAction.Add: builder.Append(" + "); break; // ArgumentOutOfRangeException.
                            case NotifyCollectionChangedAction.Remove: builder.Append(" x "); break; // ArgumentOutOfRangeException.
                            default: builder.Append(" - "); break; // ArgumentOutOfRangeException.
                        }
                        builder.AppendLine(item.Name); // ArgumentOutOfRangeException.
                    }
                    builder.AppendLine().AppendLine("Proceed with download?"); // ArgumentOutOfRangeException.

                    if (this.OnUpdatesAvailable(builder.ToString())) // ArgumentNullException
                    {
                        foreach (var item in updates) if (!item.Update()) App.Warnings.Push($"Failed to update {item.Name}."); // NotSupportedException
                    }
                }
            }
            catch (UriFormatException e) { throw new OperationFailedException(e); }
            catch (ArgumentOutOfRangeException e) { throw new ShouldNotHappenException(e); }
        }

        /// <exception cref="NotSupportedException"></exception>
        private List<GitHubFileInfo> ListBinaries()
        {
            var settings = Properties.Settings.Default;
            var assembly = System.Reflection.Assembly.GetExecutingAssembly();
            var location = assembly.Location; // NotSupportedException.
            if (String.IsNullOrWhiteSpace(location)) throw new NotSupportedException();

            var skipPredicate = new Func<String, Boolean>(
                (name) => name?.ToLowerInvariant().Trim().EndsWith(".config", StringComparison.InvariantCultureIgnoreCase) ?? false);
            return this.QuietQuery(settings.BinUrl, location, false, skipPredicate);
        }

        private List<GitHubFileInfo> ListMisc()
        {
            var settings = Properties.Settings.Default;
            var config = Bridge.Config.Instance;

            var maps = this.QuietQuery(settings.MapsUrl, config.MapsPath);
            var faces = this.QuietQuery(settings.FacesUrl, config.FacesPath);
            var skills = this.QuietQuery(settings.SkillsUrl, config.SkillsPath);

            var count = maps.Count + faces.Count + skills.Count;
            var updates = new List<GitHubFileInfo>(count);
            updates.AddRange(maps);
            updates.AddRange(faces);
            updates.AddRange(skills);

            return updates;
        }

        private List<GitHubFileInfo> QuietQuery(String url, String localFolderPath, Boolean doDelete = true, Func<String, Boolean> skipPredicate = null)
        {
            try
            {
                return this.Query(url, localFolderPath, doDelete, skipPredicate);
            }
            catch (ArgumentNullException) { return new List<GitHubFileInfo>(); }
            catch (UriFormatException) { return new List<GitHubFileInfo>(); }
            catch (OperationFailedException) { return new List<GitHubFileInfo>(); }
        }

        /// <summary>
        /// Compares the contents of a local folder to a git repository, and builds a list of changes required to update the local copy.
        /// </summary>
        /// <param name="url">Online git location.</param>
        /// <param name="localFolderPath">Local folder location, absolute or relative.</param>
        /// <param name="doDelete">Indicates if files that are present in the local folder but missing in the online repository are to be marked for deletion.</param>
        /// <returns></returns>
        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="UriFormatException"></exception>
        /// <exception cref="OperationFailedException"></exception>
        private List<GitHubFileInfo> Query(String url, String localFolderPath, Boolean doDelete, Func<String, Boolean> skipPredicate)
        {
            if (url.IsNull()) throw new ArgumentNullException(nameof(url));
            if (localFolderPath.IsNull()) throw new ArgumentNullException(nameof(localFolderPath));
            if (skipPredicate.IsNull()) skipPredicate = (name) => false;
            
            try
            {
                // Get a list of local files.
                var absoluteFolderPath = Path.GetFullPath(localFolderPath);
                if (!Directory.Exists(absoluteFolderPath))
                {
                    //App.Warnings.Push($"Invalid location '{localFolderPath}'.");
                    throw new OperationFailedException($"Invalid location '{localFolderPath}'.");
                }
                var onlineFiles = GitHubFileInfo.Get(url); // UriFormatException, OperationFailedException.
                if (onlineFiles.IsNull()) throw new ShouldNotHappenException();

                var updates = new List<GitHubFileInfo>(onlineFiles.Count);
                foreach (var online in onlineFiles)
                {
                    if (skipPredicate(online.Name)) continue;
                    var localFilePath = Path.Combine(absoluteFolderPath, online.Name);
                    if (!File.Exists(localFilePath))
                    {
                        //App.Warnings.Push($"Missing file '{online.Name}'.");
                        online.MarkForUpdate(localFilePath, NotifyCollectionChangedAction.Add);
                        updates.Add(online);
                    }
                    else
                    {
                        var localSha = GitHubFileInfo.HashFile(localFilePath).ToHex(); // OperationFailedException.
                        if (localSha != online.Sha) 
                        {
                            //App.Warnings.Push($"Update available for map {online.Name}.");
                            online.MarkForUpdate(localFilePath, NotifyCollectionChangedAction.Replace);
                            updates.Add(online);
                        }
                    }
                }

                if (doDelete)
                {
                    foreach (var localFilePath in Directory.GetFiles(absoluteFolderPath))
                    {
                        var name = Path.GetFileName(localFilePath);
                        var isDeprecated = true;
                        foreach (var online in onlineFiles) if (online.Name == name) { isDeprecated = false; break; }
                        if (isDeprecated)
                        {
                            //App.Warnings.Push($"Map deprecated: {name}.");
                            var deprecated = new GitHubFileInfo() { Name = name };
                            deprecated.MarkForUpdate(localFilePath, NotifyCollectionChangedAction.Remove);
                            updates.Add(deprecated);
                        }
                    }
                }

                return updates;
            }
            catch (ArgumentNullException e) { throw new ShouldNotHappenException(e); }
            catch (ArgumentOutOfRangeException e) { throw new ShouldNotHappenException(e); }
            catch (SecurityException e) { throw new OperationFailedException(e); }
            catch (ArgumentException e) { throw new OperationFailedException(e); }
            catch (NotSupportedException e) { throw new OperationFailedException(e); }
            catch (PathTooLongException e) { throw new OperationFailedException(e); }
            catch (DirectoryNotFoundException e) { throw new OperationFailedException(e); }
            catch (UnauthorizedAccessException e) { throw new OperationFailedException(e); }
            catch (IOException e) { throw new OperationFailedException(e); }
        }
    }
}
