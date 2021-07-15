using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Hosting;

namespace PolycomWebApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            CreateHostBuilder(args).Build().Run();
        }

        public static IHostBuilder CreateHostBuilder(string[] args) =>
            Host.CreateDefaultBuilder(args)
                .ConfigureWebHostDefaults(webBuilder =>
                {
                    webBuilder.UseKestrel(kso => kso.ListenAnyIP(5000));
                    webBuilder.UseStartup<Startup>();
                });
    }
}
