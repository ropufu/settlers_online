using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Net;
using System.Security;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.LeytePond
{
    // https://developer.github.com/v3/repos/contents/
    [JsonObject(MemberSerialization.OptIn)]
    public class GitHubFileInfo
    {
        [JsonProperty("name")]
        private String name = String.Empty;
        [JsonProperty("path")]
        private String relativePath = String.Empty;
        [JsonProperty("sha")]
        private String sha = String.Empty;
        [JsonProperty("size")]
        private Int32 sizeInBytes = 0;
        [JsonProperty("url")]
        private String apiUrl = String.Empty;
        [JsonProperty("html_url")]
        private String htmlUrl = String.Empty;
        [JsonProperty("git_url")]
        private String gitUrl = String.Empty;
        [JsonProperty("download_url")]
        private String downloadUrl = String.Empty;
        [JsonProperty("type")]
        private String fileType = String.Empty;

        private String localPath = null;
        private NotifyCollectionChangedAction updateAction = NotifyCollectionChangedAction.Reset;

        public String Name { get => this.name; set => this.name = value; }
        public String Sha => this.sha;
        public Int32 SizeInBytes => this.sizeInBytes;

        public String LocalPath => this.localPath;
        public NotifyCollectionChangedAction UpdateAction => this.updateAction;

        /// <exception cref="ArgumentNullException"></exception>
        private static void FilterNewLine(List<Byte> buffer)
        {
            if (buffer.IsNull()) throw new ArgumentNullException(nameof(buffer));
            for (var i = 1; i < buffer.Count; ++i)
            {
                var isNewLine = ((buffer[i - 1] == '\r') && (buffer[i] == '\n'));
                if (isNewLine)
                {
                    buffer.RemoveAt(i - 1); // Remove the '\r'.
                    --i; // Decrement the current position.
                }
            }
        }

        /// <exception cref="ArgumentNullException"></exception>
        private static Boolean IsTextFile(String filePath, Boolean doCheckContent = false)
        {
            if (filePath.IsNull()) throw new ArgumentNullException(nameof(filePath));

            filePath = filePath.ToLowerInvariant().Trim();
            var textExtensions = new String[] { ".txt", ".json", ".config", ".manifest", ".xml", ".xaml", ".h", ".hpp", ".c", ".cpp", ".cs", ".bat", ".tex", ".log" };
            foreach (var extension in textExtensions)
            {
                if (filePath.EndsWith(extension)) return true;
            }

            if (doCheckContent)
            {
                try
                {
                    using (var stream = File.OpenRead(filePath))
                    {
                        var c = stream.ReadByte();
                        while (true)
                        {
                            if (c == -1) return true; // If the end of file has been reached, this is apparently a text file.
                            // If any non-printable character is encountered, this is apparently not a text file.
                            if (c > 126) return false;
                            if (c < 32 && c != 0) return false;
                            c = stream.ReadByte();
                        }
                    }
                }
                catch (ArgumentNullException) { throw new ShouldNotHappenException(); }
                catch (ObjectDisposedException) { }
                catch (ArgumentException) { }
                catch (PathTooLongException) { }
                catch (DirectoryNotFoundException) { }
                catch (UnauthorizedAccessException) { }
                catch (FileNotFoundException) { }
                catch (NotSupportedException) { }
                catch (IOException) { }
            }
            return false;
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="OperationFailedException"></exception>
        public static Byte[] HashFile(String filePath, String type = "blob")
        {
            if (filePath.IsNull()) throw new ArgumentNullException(nameof(filePath));
            if (type.IsNull()) throw new ArgumentNullException(nameof(type));

            try
            {
                var isText = GitHubFileInfo.IsTextFile(filePath);

                var fileBytes = new List<Byte>(File.ReadAllBytes(filePath));
                if (isText) GitHubFileInfo.FilterNewLine(fileBytes);
                var gitPrefixString = $"{type} {fileBytes.Count}\0";
                var gitPrefixBuffer = Encoding.ASCII.GetBytes(gitPrefixString);
                fileBytes.InsertRange(0, gitPrefixBuffer);
                var buffer = fileBytes.ToArray();

                var sha = new System.Security.Cryptography.SHA1Managed();
                return sha.ComputeHash(buffer);
            }
            catch (ArgumentNullException e) { throw new ShouldNotHappenException(e); }
            catch (EncoderFallbackException e) { throw new OperationFailedException(e); }
            catch (ObjectDisposedException e) { throw new OperationFailedException(e); }
            catch (SecurityException e) { throw new OperationFailedException(e); }
            catch (ArgumentException e) { throw new OperationFailedException(e); }
            catch (NotSupportedException e) { throw new OperationFailedException(e); }
            catch (PathTooLongException e) { throw new OperationFailedException(e); }
            catch (DirectoryNotFoundException e) { throw new OperationFailedException(e); }
            catch (UnauthorizedAccessException e) { throw new OperationFailedException(e); }
            catch (FileNotFoundException e) { throw new OperationFailedException(e); }
            catch (InvalidOperationException e) { throw new OperationFailedException(e); }
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentOutOfRangeException"></exception>
        public void MarkForUpdate(String localPath, NotifyCollectionChangedAction updateAction)
        {
            if (localPath.IsNull()) throw new ArgumentNullException(nameof(localPath));

            switch (updateAction)
            {
                case NotifyCollectionChangedAction.Add:
                case NotifyCollectionChangedAction.Remove:
                case NotifyCollectionChangedAction.Replace:
                    break;
                default: throw new ArgumentOutOfRangeException(nameof(updateAction));
            }

            this.localPath = localPath;
            this.updateAction = updateAction;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <exception cref="NotSupportedException"></exception>
        public Boolean Update()
        {
            if (this.localPath.IsNull()) throw new NotSupportedException();
            if (this.downloadUrl.IsNull()) throw new NotSupportedException();

            switch (this.updateAction)
            {
                case NotifyCollectionChangedAction.Add:
                case NotifyCollectionChangedAction.Replace:
                    try
                    {
                        var request = (HttpWebRequest)WebRequest.Create(this.downloadUrl);
                        request.AutomaticDecompression = DecompressionMethods.GZip; // InvalidOperationException.
                        request.UserAgent = System.Reflection.Assembly.GetExecutingAssembly().FullName;

                        using (var response = (HttpWebResponse)request.GetResponse())
                        {
                            using (var stream = response.GetResponseStream())
                            using (var fileStream = File.Create(this.localPath))
                            {
                                stream.CopyTo(fileStream);
                            }
                        }
                        this.updateAction = NotifyCollectionChangedAction.Reset;
                        return true;
                    }
                    catch (ArgumentNullException e) { throw new ShouldNotHappenException(e); }
                    catch (ArgumentException) { }
                    catch (PathTooLongException) { }
                    catch (DirectoryNotFoundException) { }
                    catch (ProtocolViolationException) { }
                    catch (NotSupportedException) { }
                    catch (SecurityException) { }
                    catch (UriFormatException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (WebException) { }
                    catch (ObjectDisposedException) { }
                    catch (InvalidOperationException) { }
                    catch (IOException) { }
                    return false;
                case NotifyCollectionChangedAction.Remove:
                    try
                    {
                        File.Delete(this.localPath);
                        this.updateAction = NotifyCollectionChangedAction.Reset;
                        return true;
                    }
                    catch (ArgumentNullException e) { throw new ShouldNotHappenException(e); }
                    catch (ArgumentException) { }
                    catch (DirectoryNotFoundException) { }
                    catch (PathTooLongException) { }
                    catch (NotSupportedException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (IOException) { }
                    return false;
                default: return true;
            }
        }

        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="UriFormatException"></exception>
        /// <exception cref="OperationFailedException"></exception>
        public static List<GitHubFileInfo> Get(String url)
        {
            if (url.IsNull()) throw new ArgumentNullException(nameof(url));

            var files = new List<GitHubFileInfo>();
            try
            {
                var request = (HttpWebRequest)WebRequest.Create(url); // UriFormatException.
                if (request.IsNull()) throw new OperationFailedException();

                request.AutomaticDecompression = DecompressionMethods.GZip;
                request.UserAgent = System.Reflection.Assembly.GetExecutingAssembly().FullName;

                using (var response = (HttpWebResponse)request.GetResponse())
                {
                    using (var reader = new StreamReader(response.GetResponseStream()))
                    {
                        var json = reader.ReadToEnd();
                        files = JsonConvert.DeserializeObject<List<GitHubFileInfo>>(json);
                    }
                }

                return files;
            }
            catch (ArgumentNullException e) { throw new ShouldNotHappenException(e); }
            catch (OutOfMemoryException e) { throw new OperationFailedException(e); }
            catch (IOException e) { throw new OperationFailedException(e); }
            catch (InvalidCastException e) { throw new OperationFailedException(e); }
            catch (ProtocolViolationException e) { throw new OperationFailedException(e); }
            catch (WebException e) { throw new OperationFailedException(e); }
            catch (ObjectDisposedException e) { throw new OperationFailedException(e); }
            catch (InvalidOperationException e) { throw new OperationFailedException(e); }
            catch (SecurityException e) { throw new OperationFailedException(e); }
            catch (NotSupportedException e) { throw new OperationFailedException(e); }
        }
    }
}
