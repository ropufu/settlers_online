using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Net;
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
        public NotifyCollectionChangedAction UpdateAction => this.updateAction;

        public static Byte[] HashFile(String filePath)
        {
            var fileString = System.IO.File.ReadAllText(filePath).Replace("\r\n", "\n");
            var gitString = $"blob {fileString.Length}\0{fileString}";

            var sha = new System.Security.Cryptography.SHA1Managed();
            var buff = Encoding.ASCII.GetBytes(gitString);
            return sha.ComputeHash(buff);
        }

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

        public Boolean Update()
        {
            if (this.localPath.IsNull()) throw new NotSupportedException();
            switch (this.updateAction)
            {
                case NotifyCollectionChangedAction.Add:
                case NotifyCollectionChangedAction.Replace:
                    try
                    {
                        var request = (HttpWebRequest)WebRequest.Create(this.downloadUrl);
                        request.AutomaticDecompression = DecompressionMethods.GZip;
                        request.UserAgent = System.Reflection.Assembly.GetExecutingAssembly().FullName;

                        using (var response = (HttpWebResponse)request.GetResponse())
                        {
                            using (var stream = response.GetResponseStream())
                            using (var fileStream = System.IO.File.Create(this.localPath))
                            {
                                stream.CopyTo(fileStream);
                            }
                        }
                        this.updateAction = NotifyCollectionChangedAction.Reset;
                        return true;
                    }
                    catch (UnauthorizedAccessException) { }
                    catch (System.IO.IOException) { }
                    return false;
                case NotifyCollectionChangedAction.Remove:
                    try
                    {
                        System.IO.File.Delete(this.localPath);
                        this.updateAction = NotifyCollectionChangedAction.Reset;
                        return true;
                    }
                    catch (UnauthorizedAccessException) { }
                    catch (System.IO.IOException) { }
                    return false;
                default: return true;
            }
        }

        public static List<GitHubFileInfo> Get(String url)
        {
            var files = new List<GitHubFileInfo>();

            var request = (HttpWebRequest)WebRequest.Create(url);
            request.AutomaticDecompression = DecompressionMethods.GZip;
            request.UserAgent = System.Reflection.Assembly.GetExecutingAssembly().FullName;

            using (var response = (HttpWebResponse)request.GetResponse())
            {
                using (var reader = new System.IO.StreamReader(response.GetResponseStream()))
                {
                    var json = reader.ReadToEnd();
                    files = JsonConvert.DeserializeObject<List<GitHubFileInfo>>(json);
                }
            }

            return files;
        }
    }
}
