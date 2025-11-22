#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMap>
#include <QDateTime>

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

private:
    QStackedWidget *stack;
    QPushButton *chatsButton;
    QPushButton *settingsButton;
    QPushButton *profileButton;

    QWidget *chatPage;
    QVBoxLayout *messageLayout;
    QLineEdit *messageEdit;

    QList<QString> meMessages;
    QMap<QString, QDateTime> chatActivity;

    QWidget *createChatsPage();
    QWidget *createSettingsPage();
    QWidget *createProfilePage();
    void refreshChatList();
};
