using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json;

namespace lord13;

public class User //common
{
    public string login { get; set; }
    public string loginHash { get; private set; }
    public string passwordHash { get; set; }

    public User(string login, string password)
    {
        this.login = login;
        loginHash = Hash(login);
        passwordHash = Hash(password);
    }

    private string Hash(string name)
    {
        if (string.IsNullOrEmpty(name))
            return "0";

        string hash = "";

        for (int i = 0; i < name.Length; ++i)
        {
            int c = (int)(name[i]);
            hash += System.Convert.ToString(Math.Pow(c, i + 1));
        }
        return hash;
    }
}

public class UserMeth //methods
{
    public string usersFile = "susers.json";
    private List<User> users = new List<User>();
    public bool URegister(User user)
    {
        if (!LUnique(user.login) || user.login.Length > 16 || user.login.Length < 3)
        {
            Service.Error();
            return false;
        }
        
        users.Add(user);
        SaveU(user);
        return true;
    }

    public bool UAuth(string login, string password)
    {
        User usero = users.Find(u => u.login == login);
        if (usero == null) 
            return false;

        User tempUser = new User("", password);
        
        return tempUser.passwordHash == tempUser.passwordHash;
    }

    public bool LUnique(string login)
    {
        return users.Find(u => u.login == login) == null;
    }

    public void LoadU()
    {
        string[] lines = File.ReadAllLines(usersFile);
        foreach (string line in lines)
        {
            User user = JsonConvert.DeserializeObject<User>(line);
            users.Add(user);
        }
    }
    
    public void SaveU(User user)
    {
        string json = JsonConvert.SerializeObject(user);
        File.AppendAllText(usersFile, json + Environment.NewLine);
    }
}