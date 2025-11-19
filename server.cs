using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

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

public class ChatMessage
{
    private string type { get; set; }
    private string text { get; set; }
    private string userName { get; set; }
    private DateTime Timestamp { get; set; }

    public ChatMessage(string type, string userName, string text)
    {
        this.type = type;
        this.userName = userName;
        this.text = text;
        Timestamp = DateTime.Now;
    }
}