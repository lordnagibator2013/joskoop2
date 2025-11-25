using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.IO;
using System.Security.Policy;
using System.ComponentModel.DataAnnotations;
using static System.Math;

namespace lord13;

public class ChatServer
{
    private TcpListener listener;
    private bool isRunning;
    private List<TcpClient> connectedClients = new List<TcpClient>();
    private List<ChatMessage> messageHistory = new List<ChatMessage>();
    private string messageFile = "smessage.json";

    public ChatServer(int port = 8888)
    {
        listener = new TcpListener(IPAddress.Any, port);
        connectedClients = new List<TcpClient>();
        LoadHistory();
    }

    public async Task StartAsync()
    {
        listener.Start();
        isRunning = true;
        Console.WriteLine("нападайте");

        while (isRunning)
        {
            try
            {
                TcpClient client = await listener.AcceptTcpClientAsync();
                Console.WriteLine("это кто");
                
                string userName = await ReceiveStringAsync(client.GetStream());
                Console.WriteLine($"это {userName}");
                
                _ = HandleClientAsync(client, userName);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"пизда {ex.Message}");
            }
        }
    }

    private async Task<string> ReceiveStringAsync(NetworkStream stream)
    {
        byte[] buffer = new byte[4096];
        int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
        return Encoding.UTF8.GetString(buffer, 0, bytesRead);
    }

    private Task HandleClientAsync(TcpClient client)
    {
        NetworkStream stream = client.GetStream();
        byte[] buffer = new byte[4096];

        Console.WriteLine($"бродяга {userName} явился к ещё {connectedClients.Count - 1}");
        
        lock (connectedClients)
        {
            connectedClients.Add(client);
        }
        
        try
        {
            while (client.Connected)
            {
                // Читаем сообщение от клиента
                int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
                if (bytesRead == 0) break; // Клиент отключился
                
                string json = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                
                // Создаём объект сообщения
                ChatMessage message = JsonConvert.DeserializeObject<ChatMessage>(json);
                message.UserName = userName;
                message.Timestamp = DateTime.Now;
                
                // Сохраняем и рассылаем
                SaveMessage(message);
                await BroadcastToAllClients(message);
                
                Console.WriteLine($"[{message.Timestamp}] {userName}: {message.Text}");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"че с тобой {userName} у тебя {ex.Message}");
        }
        finally
        {
            lock (connectedClients)
            {
                connectedClients.Remove(client);
            }
            client.Close();
            Console.WriteLine($"Клиент {userName} отключился. Осталось: {connectedClients.Count}");
        }
    }

    private void LoadHistory()
    {
        if (File.Exists(messageFile))
        {
            try
            {
                string json = File.ReadAllText(messageFile);
                messageHistory = JsonConvert.DeserializeObject<List<ChatMessage>>(json) ?? new List<ChatMessage>();
                Console.WriteLine($"стока {messageHistory.Count} сообщений ");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"дем {ex.Message}");
                messageHistory = new List<ChatMessage>();
            }
        }
        else
        {
            messageHistory = new List<ChatMessage>();
        }
    }

    public void SaveMessage(ChatMessage message)
    {
        messageHistory.Add(message);
        string json = JsonConvert.SerializeObject(messageHistory, Formatting.Indented);
        File.WriteAllText(messageFile, json);
    }

    private async Task BroadcastToAllClients(ChatMessage message)
    {
        string json = JsonConvert.SerializeObject(message);
        byte[] data = Encoding.UTF8.GetBytes(json);
        
        List<TcpClient> clientsToBroadcast;
        lock (connectedClients)
        {
            clientsToBroadcast = new List<TcpClient>(connectedClients);
        }
        
        foreach (var client in clientsToBroadcast)
        {
            if (client.Connected)
            {
                try
                {
                    await client.GetStream().WriteAsync(data, 0, data.Length);
                }
                catch
                {
                    connectedClients.Remove(client);
                }
            }
        }
    }
}

public class ChatMessage // soobsch
{
    public string type { get; set; }
    public string text { get; set; }
    public string userName { get; set; }
    public DateTime Timestamp { get; set; }

    public ChatMessage(string type, string userName, string text)
    {
        this.type = type;
        this.userName = userName;
        this.text = text;
        Timestamp = DateTime.Now;
    }
}