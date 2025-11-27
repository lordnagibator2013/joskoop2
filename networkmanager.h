#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void connectToServer(const QString &host, quint16 port, const QString &userName);
    void sendMessage(const QString &text);
    bool isConnected() const;
    void disconnectFromServer();

    void setUserName(const QString &userName);
    QString getUserName() const;

signals:
    void messageReceived(const QString &sender, const QString &text, bool isOwnMessage);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *socket;
    QString userName;
};
