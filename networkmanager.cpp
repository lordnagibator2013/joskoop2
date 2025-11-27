#include "networkmanager.h"
#include <QHostAddress>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkManager::onError);

    qDebug() << "NetworkManager initialized";
}

NetworkManager::~NetworkManager()
{
    disconnectFromServer();
}

void NetworkManager::connectToServer(const QString &host, quint16 port, const QString &userName)
{
    this->userName = userName;
    qDebug() << "Connecting to server:" << host << "port:" << port << "as user:" << userName;
    socket->connectToHost(host, port);
}

void NetworkManager::disconnectFromServer()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Disconnecting from server";
        socket->disconnectFromHost();
    }
}

void NetworkManager::sendMessage(const QString &text)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send message - not connected to server";
        emit errorOccurred("Not connected to server");
        return;
    }

    QJsonObject message;
    message["type"] = "Message";
    message["userName"] = userName;
    message["text"] = text;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(message);
    QByteArray data = doc.toJson() + "\n";

    qDebug() << "Sending message:" << text;
    qDebug() << "JSON data:" << data;

    qint64 bytesWritten = socket->write(data);

    if (bytesWritten == -1) {
        QString error = "Failed to send message: " + socket->errorString();
        qDebug() << error;
        emit errorOccurred(error);
    } else {
        socket->flush();
        qDebug() << "Message sent successfully, bytes written:" << bytesWritten;
    }
}

bool NetworkManager::isConnected() const
{
    bool connected = (socket->state() == QAbstractSocket::ConnectedState);
    qDebug() << "Connection status:" << (connected ? "Connected" : "Disconnected");
    return connected;
}

void NetworkManager::setUserName(const QString &userName)
{
    this->userName = userName;
    qDebug() << "Username set to:" << userName;
}

QString NetworkManager::getUserName() const
{
    return userName;
}

void NetworkManager::onConnected()
{
    qDebug() << "Successfully connected to server";

    // Отправляем информацию о пользователе при подключении
    QJsonObject connectMsg;
    connectMsg["type"] = "Connect";
    connectMsg["userName"] = userName;

    QJsonDocument doc(connectMsg);
    QByteArray data = doc.toJson() + "\n";

    qDebug() << "Sending connection message:" << data;

    qint64 bytesWritten = socket->write(data);
    if (bytesWritten > 0) {
        socket->flush();
        qDebug() << "Connection message sent successfully";
    } else {
        qDebug() << "Failed to send connection message";
    }

    emit connectionStatusChanged(true);
    qDebug() << "Connected to server as:" << userName;
}

void NetworkManager::onDisconnected()
{
    qDebug() << "Disconnected from server";
    emit connectionStatusChanged(false);
}

void NetworkManager::onReadyRead()
{
    qDebug() << "Data available for reading, bytes available:" << socket->bytesAvailable();

    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        data = data.trimmed();

        qDebug() << "Raw data received:" << data;

        if (data.isEmpty()) {
            qDebug() << "Empty data received, skipping";
            continue;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qDebug() << "JSON parse error:" << error.errorString() << "at offset:" << error.offset;
            qDebug() << "Problematic data:" << data;
            continue;
        }

        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "Invalid JSON object or null document";
            qDebug() << "Data:" << data;
            continue;
        }

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();
        QString sender = obj["userName"].toString(); // Важно: userName
        QString text = obj["text"].toString();
        QString timestamp = obj["timestamp"].toString();

        qDebug() << "Parsed message - Type:" << type
                 << "From:" << sender
                 << "Text:" << text
                 << "Timestamp:" << timestamp;

        if (type == "Message" && !text.isEmpty()) {
            // Определяем, является ли сообщение входящим или исходящим
            bool isOwnMessage = (sender == userName);

            qDebug() << "Message details - Our user:" << userName
                     << "Sender:" << sender
                     << "IsOwn:" << isOwnMessage;

            qDebug() << "Emitting messageReceived signal";
            emit messageReceived(sender, text, isOwnMessage);
        } else if (type == "Connect") {
            qDebug() << "Connection message received from:" << sender;
        } else {
            qDebug() << "Unknown message type or empty text:" << type;
        }
    }

    // Если остались данные, но нет полной строки
    if (socket->bytesAvailable() > 0) {
        qDebug() << "Incomplete data remaining, waiting for more...";
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError error)
{
    QString errorString = socket->errorString();
    qDebug() << "Socket error:" << error << "-" << errorString;
    emit errorOccurred(errorString);
}
