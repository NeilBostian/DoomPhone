using System.Net.Http;
using System.Threading.Tasks;

namespace PolycomFinal
{
    class Program
    {
        static async Task Main(string[] args)
        {
            // Not sure if the multipart boundry makes any difference... probably doesn't matter
            var httpContent = new MultipartFormDataContent("---------------------------303889579422103152842123901125");
            byte[] contentBytes = System.Text.Encoding.UTF8.GetBytes("Hello, world! From a text file.");

            // The form name is relevant if you use the UI to add/delete files,
            // but if you just want to upload arbitrary files the form name doesn't matter
            string formName = "testFormName";

            // File name to upload
            string fileName = "helloworld.txt";

            httpContent.Add(new ByteArrayContent(contentBytes), formName, fileName);

            var httpRequest = new HttpRequestMessage(HttpMethod.Post, "http://192.168.1.4/form-submit/Preferences/Background/upload");
            httpRequest.Headers.Add("Cookie", "Authorization=Basic UG9seWNvbTo0NTY="); // Username 'Polycom', password '456'
            httpRequest.Content = httpContent;

            var httpClient = new HttpClient();
            var response = await httpClient.SendAsync(httpRequest);
            var responseContent = await response.Content.ReadAsStringAsync();
        }
    }
}
