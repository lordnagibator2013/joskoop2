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
    private List<TcpClient> connectedClients;
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

        while (isRunning)
        {
            try
            {
                TcpClient client = await listener.AcceptTcpClientAsync();
                connectedClients.Add(client);
                Console.WriteLine($"ещё один, всего бродяг {connectedClients.Count}");

                _ = HandleClientAsync(client);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"свонялся: {ex.Message}");
            }
        }
    }

    private Task HandleClientAsync(TcpClient client)
    {
        return Task.CompletedTask;
    }

    private void LoadHistory()
    {
        string[] lines = File.ReadAllLines(messageFile);
        foreach (string line in lines)
        {
            if (!string.IsNullOrEmpty(line))
            {
                ChatMessage message = JsonConvert.DeserializeObject<ChatMessage>(line);
                messageHistory.Add(message);
            }
        }
    }

    public void SaveMessage(ChatMessage message)
    {
        string json = JsonConvert.SerializeObject(message);
        File.AppendAllText(messageFile, json + Environment.NewLine);
        messageHistory.Add(message);
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