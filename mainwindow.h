#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMap>
#include <QDateTime>
#include <QLabel>
#include <QAbstractAnimation>

class NetworkManager;

struct ChatMessage {
    QString text;
    bool isOutgoing;
    QString sender;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(NetworkManager *netManager, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void switchToChats();
    void switchToSettings();
    void switchToProfile();
    void openMeChat();
    void sendMessage();
    void receiveMessage(const QString &text, bool isOutgoing, const QString &sender); // 3 параметра
    void chooseChatWallpaper();
    void onNetworkMessageReceived(const QString &sender, const QString &text, bool isOwnMessage); // 3 параметра
    void onNetworkError(const QString &error);
    void showEmojiPicker();

private:
    NetworkManager *networkManager;
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

    QWidget* createMessageBubble(const QString &text, bool isOutgoing, const QString &sender = "");
    void animateMessage(QWidget *target);
    QLabel *chatBackgroundLabel = nullptr;
    QString selectedWallpaperPath;
    QList<QAbstractAnimation*> activeAnimations;
    bool isEmojiOnly(const QString &text);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};
