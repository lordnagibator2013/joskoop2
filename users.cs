using System;

namespace lord13;

public class User //common
{
    private string login { get; set; }
    public string loginHash { get; set; }
    private string passwordHash { get; set; }

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
    public string[] users;
    public void URegister(User user)
    {

    }

    public void UAuth(User user)
    {

    }

    public bool LUnique(string login)
    {
        bool unique = true;

        return unique;
    }

    public void LoadU()
    {
        string[] lines = File.ReadAllLines(usersFile);
        foreach (string line in lines)
        {
            User user = JsonConvert.DeserializeObject<User>(line);
            _users.Add(user);
        }
    }
    
    public void SaveU()
    {
        
    }
}