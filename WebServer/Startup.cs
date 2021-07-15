using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using System.IO;

namespace PolycomWebApp
{
    public class Startup
    {
        public void ConfigureServices(IServiceCollection services) { }

        // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
        public void Configure(IApplicationBuilder app, IWebHostEnvironment env)
        {
            app.UseRouting();

            app.UseEndpoints(endpoints =>
            {
                endpoints.MapGet("/", async context =>
                {
                    context.Response.Headers["Content-Type"] = "text/html";
                    var content = await File.ReadAllTextAsync("page.html");
                    await context.Response.WriteAsync(content);
                });

                endpoints.MapGet("/jquery", async context =>
                {
                    context.Response.Headers["Content-Type"] = "application/javascript";
                    var content = await File.ReadAllTextAsync("jquery.js");
                    await context.Response.WriteAsync(content);
                });
            });
        }
    }
}
