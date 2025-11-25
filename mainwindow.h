#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMap>
#include <QDateTime>

struct ChatMessage {
    QString text;
    bool isOutgoing;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void switchToChats();
    void switchToSettings();
    void switchToProfile();
    void openMeChat();
    void sendMessage();
    void receiveMessage(const QString &text);

private:
    QStackedWidget *stack;
    QPushButton *chatsButton;
    QPushButton *settingsButton;
    QPushButton *profileButton;

    QWidget *chatPage;
    QVBoxLayout *messageLayout;
    QLineEdit *messageEdit;

    QList<ChatMessage> chatHistory;
    QMap<QString, QDateTime> chatActivity;

    QWidget *createChatsPage();
    QWidget *createSettingsPage();
    QWidget *createProfilePage();
    void refreshChatList();
    bool hasReceivedInitialMessage = false;

    QWidget* createMessageBubble(const QString &text, bool isOutgoing);
    void animateMessage(QWidget *target);
};
