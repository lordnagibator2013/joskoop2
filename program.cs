using System;
using System.Threading.Tasks;

namespace lord13;

class Program
{
    static async Task Main()
    {
        Console.WriteLine("ку");

        ChatServer server = new ChatServer();
        ChatMessage testMsg = new ChatMessage("System", "Сервер", "Запуск");
        server.SaveMessage(testMsg);
        Console.WriteLine("ебашь");

        Console.ReadLine();
        _ = server.StartAsync();

        Console.WriteLine("P для закругления");
        while (true)
        {
            var key = Console.ReadKey();
            ServerSystem.Stop(key);
        }
    }
}

public class ServerSystem
{
    public static void Stop(ConsoleKeyInfo key)
    {
        if(key.Key == ConsoleKey.P)
        {
            Console.WriteLine("закругляемся");
            Environment.Exit(0);
        }
    }
}