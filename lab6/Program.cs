using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Web;

namespace lab6
{
    class Program
    {
        static void Main(string[] args)
        {
            // Create a listener.
            var listener = new HttpListener();
            // Add the prefixes.
            var baseUrl = "http://localhost:5000";
            var pathIndex = "/";
            var pathCallback = "/callback/";
            listener.Prefixes.Add(baseUrl + pathIndex);
            listener.Prefixes.Add(baseUrl + pathCallback);

            listener.Start();
            Console.WriteLine("Listening...");

            while (true)
            {                
                var context = listener.GetContext();

                if (context.Request.Url.AbsolutePath == pathIndex)
                {
                    // Obtain a response object.
                    var response = context.Response;
                    // Construct a response.
                    var path = "./index.html";
                    string responseString = File.ReadAllText(path);
                    byte[] buffer = System.Text.Encoding.UTF8.GetBytes(responseString);
                    // Get a response stream and write the response to it.
                    response.ContentType = "text/html";
                    response.ContentLength64 = buffer.Length;
                    var output = response.OutputStream;
                    output.Write(buffer, 0, buffer.Length);
                    // You must close the output stream.
                    output.Close();
                }
                else if (context.Request.Url.AbsolutePath == pathCallback.TrimEnd('/'))
                {
                    var request = context.Request;
                    using var reader = new StreamReader(request.InputStream, request.ContentEncoding);
                    var formData = HttpUtility.ParseQueryString(reader.ReadToEnd());
                    var mytext = formData["my-text"];
                    var result = mytext.Split(' ').Aggregate(0, (num, word) => word.Length >= 5 ? num + 1 : num);

                    var response = context.Response;
                    byte[] buffer = System.Text.Encoding.UTF8.GetBytes(result.ToString());
                    response.ContentLength64 = buffer.Length;
                    var output = response.OutputStream;
                    output.Write(buffer, 0, buffer.Length);
                }
            }
        }
    }
}
