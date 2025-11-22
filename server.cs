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
    private TcpListener _listener;
    private bool _isRunning;
    private List<TcpClient> _connectedClients;

    public ChatServer(int port = 8888)
    {
        _listener = new TcpListener(IPAddress.Any, port);
        _connectedClients = new List<TcpClient>();
    }

    public async Task StartAsync()
    {
        _listener.Start();
        _isRunning = true;

        while (_isRunning)
        {
            try
            {
                TcpClient client = await _listener.AcceptTcpClientAsync();
                _connectedClients.Add(client);
                Console.WriteLine($"ещё один, всего бродяг {_connectedClients.Count}");

                _ = HandleClientAsync(client);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"свонялся: {ex.Message}");
            }
        }
    }

    private async Task HandleClientAsync(TcpClient client)
    {

    }
}

public class ChatMessage // soobsch
{
    private string type { get; set; }
    private string text { get; set; }
    private string userName { get; set; }
    private DateTime Timestamp { get; set; }
    private string messageFile = "smessage.json";

    public ChatMessage(string type, string userName, string text)
    {
        this.type = type;
        this.userName = userName;
        this.text = text;
        Timestamp = DateTime.Now;
    }

    private void LoadHistory()
    {
        if (!messageFile)
        {
            return;
        }

        string[] lines = File.ReadAllLines(messageFile);
        foreach (string line in lines)
        {
            // тут обратно
        }
    }

    public void SaveMessage(ChatMessage message)
    {
        string json = JsonConvert.SerializeObject(message);
        File.AppendAllText(_historyFile, json + Environment.NewLine);
        _messageHistory.Add(message);
    }
}