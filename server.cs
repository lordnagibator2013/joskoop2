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
        Console.WriteLine($"Сервер запущен на порту {((IPEndPoint)_listener.LocalEndpoint).Port}...");
        
        while (_isRunning)
        {
            try
            {
                TcpClient client = await _listener.AcceptTcpClientAsync();
                _connectedClients.Add(client);
                Console.WriteLine($"Новое подключение! Всего клиентов: {_connectedClients.Count}");
                
                _ = HandleClientAsync(client);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка при подключении: {ex.Message}");
            }
        }
    }
    
    private async Task HandleClientAsync(TcpClient client)
    {
        
    }
}