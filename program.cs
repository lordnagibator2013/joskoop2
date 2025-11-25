using System;
using System.Threading.Tasks;

namespace lord13;

class Program
{
    static async Task Main()
    {
        ChatServer server = new ChatServer();
        await server.StartAsync();
    }
}