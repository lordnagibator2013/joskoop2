#include "mainwindow.h"
#include "networkmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QDateTime>
#include <QScrollArea>
#include <QScrollBar>
#include <algorithm>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QEvent>
#include <QDebug>

MainWindow::MainWindow(NetworkManager *netManager, QWidget *parent)
    : QMainWindow(parent), networkManager(netManager)
{
    setWindowTitle("Messenger - " + networkManager->getUserName());
    resize(800, 600);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Верхняя панель
    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("TopBar");
    topBar->setFixedHeight(60);

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 10, 10, 10);
    topLayout->setSpacing(10);

    QLineEdit *searchField = new QLineEdit(topBar);
    searchField->setPlaceholderText("Search...");
    searchField->setObjectName("SearchField");

    QPushButton *createChatBtn = new QPushButton(topBar);
    createChatBtn->setObjectName("CreateChatButton");
    createChatBtn->setText("+");
    createChatBtn->setFixedSize(40, 40);

    topLayout->addWidget(searchField);
    topLayout->addWidget(createChatBtn);

    // Стек
    stack = new QStackedWidget(this);
    stack->addWidget(createChatsPage());     // index 0
    stack->addWidget(createSettingsPage());  // index 1
    stack->addWidget(createProfilePage());   // index 2

    // Нижняя панель
    QWidget *bottomBar = new QWidget(this);
    bottomBar->setObjectName("BottomBar");
    bottomBar->setFixedHeight(60);

    QHBoxLayout *navLayout = new QHBoxLayout(bottomBar);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(30);
    navLayout->setAlignment(Qt::AlignCenter);

    chatsButton = new QPushButton("Chats", bottomBar);
    chatsButton->setObjectName("NavChats");
    chatsButton->setFixedSize(100, 30);

    settingsButton = new QPushButton("Settings", bottomBar);
    settingsButton->setObjectName("NavSettings");
    settingsButton->setFixedSize(100, 30);

    profileButton = new QPushButton("Profile", bottomBar);
    profileButton->setObjectName("NavProfile");
    profileButton->setFixedSize(100, 30);

    navLayout->addWidget(chatsButton);
    navLayout->addWidget(settingsButton);
    navLayout->addWidget(profileButton);

    connect(chatsButton,   &QPushButton::clicked, this, &MainWindow::switchToChats);
    connect(settingsButton,&QPushButton::clicked, this, &MainWindow::switchToSettings);
    connect(profileButton, &QPushButton::clicked, this, &MainWindow::switchToProfile);

    // Подключаем сетевые сигналы
    if (networkManager) {
        connect(networkManager, &NetworkManager::messageReceived,
                this, &MainWindow::onNetworkMessageReceived);
        connect(networkManager, &NetworkManager::errorOccurred,
                this, &MainWindow::onNetworkError);
    }

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(stack);
    mainLayout->addWidget(bottomBar);
    setCentralWidget(central);

    // Инициализация активности и списка чатов
    chatActivity["Group Chat"] = QDateTime::currentDateTime();
    refreshChatList();
    stack->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onNetworkMessageReceived(const QString &sender, const QString &text, bool isOwnMessage)
{
    qDebug() << "Displaying message - Sender:" << sender << "Text:" << text << "IsOwn:" << isOwnMessage;

    // Если чат открыт, отображаем сообщение
    if (chatPage && stack->currentWidget() == chatPage) {
        // Для сетевых сообщений isOwnMessage = true только если это наше сообщение
        // Но в receiveMessage параметр isOutgoing = false для входящих, true для исходящих
        bool isOutgoing = isOwnMessage;
        receiveMessage(text, isOutgoing, sender);
    }

    // Обновляем активность чата
    chatActivity["Group Chat"] = QDateTime::currentDateTime();
    refreshChatList();
}

void MainWindow::onNetworkError(const QString &error)
{
    QMessageBox::warning(this, "Ошибка сети", error);
}

QWidget* MainWindow::createChatsPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    QLabel *label = new QLabel("Select a chat to start messaging:", page);
    layout->addWidget(label);

    QWidget *chatListContainer = new QWidget(page);
    chatListContainer->setObjectName("ChatList");
    QVBoxLayout *chatListLayout = new QVBoxLayout(chatListContainer);
    chatListLayout->setSpacing(10);
    chatListLayout->setContentsMargins(0, 0, 0, 0);
    chatListContainer->setLayout(chatListLayout);

    layout->addWidget(chatListContainer);
    return page;
}

QWidget* MainWindow::createSettingsPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("Settings", page);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Кнопка смены обоев
    QPushButton *wallpaperBtn = new QPushButton("Change Chat Wallpaper", page);
    wallpaperBtn->setObjectName("WallpaperButton");
    wallpaperBtn->setFixedHeight(40);
    layout->addWidget(wallpaperBtn);

    connect(wallpaperBtn, &QPushButton::clicked, this, &MainWindow::chooseChatWallpaper);

    layout->addStretch();
    return page;
}

QWidget* MainWindow::createProfilePage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);

    QString userName = networkManager ? networkManager->getUserName() : "Unknown";
    QLabel *title = new QLabel("Profile: " + userName, page);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    layout->addStretch();
    return page;
}

void MainWindow::refreshChatList() {
    QWidget *chatsPage = stack->widget(0);
    QWidget *chatListContainer = chatsPage->findChild<QWidget*>("ChatList");
    if (!chatListContainer) return;

    QLayout *layout = chatListContainer->layout();
    while (QLayoutItem *child = layout->takeAt(0)) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    QList<QString> sortedChats = chatActivity.keys();
    std::sort(sortedChats.begin(), sortedChats.end(), [&](const QString &a, const QString &b) {
        return chatActivity[a] > chatActivity[b];
    });

    for (const QString &chatName : sortedChats) {
        QPushButton *chatBtn = new QPushButton(chatName, chatListContainer);
        chatBtn->setObjectName("ChatEntry");
        chatBtn->setFixedHeight(40);
        layout->addWidget(chatBtn);

        if (chatName == "Group Chat") {
            connect(chatBtn, &QPushButton::clicked, this, &MainWindow::openMeChat);
        }
    }

    if (auto *vbox = qobject_cast<QVBoxLayout*>(layout)) {
        vbox->addStretch();
    }
}

void MainWindow::openMeChat() {
    chatPage = new QWidget(this);
    chatPage->setObjectName("ChatPage");
    chatPage->installEventFilter(this);
    chatPage->setAttribute(Qt::WA_StyledBackground, true);

    // Фоновое изображение
    if (!selectedWallpaperPath.isEmpty()) {
        chatBackgroundLabel = new QLabel(chatPage);
        chatBackgroundLabel->setObjectName("ChatBackground");
        chatBackgroundLabel->setScaledContents(true);
        chatBackgroundLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        chatBackgroundLabel->lower();
        chatBackgroundLabel->resize(chatPage->size());
        chatBackgroundLabel->move(0, 0);

        QImage img;
        if (img.load(selectedWallpaperPath)) {
            QPixmap pm = QPixmap::fromImage(img);
            QPixmap scaled = pm.scaled(chatPage->size(),
                                       Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);
            chatBackgroundLabel->setPixmap(scaled);
        }
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(chatPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Верхняя панель
    QWidget *chatTopBar = new QWidget(chatPage);
    chatTopBar->setObjectName("ChatTopBar");
    chatTopBar->setFixedHeight(50);

    QHBoxLayout *topLayout = new QHBoxLayout(chatTopBar);
    topLayout->setContentsMargins(10, 10, 10, 10);
    topLayout->setSpacing(10);

    QPushButton *backBtn = new QPushButton(chatTopBar);
    backBtn->setObjectName("BackButton");
    backBtn->setText("← Back");
    backBtn->setFixedSize(80, 30);

    QLabel *chatTitle = new QLabel("Group Chat", chatTopBar);
    chatTitle->setObjectName("ChatTitle");
    QFont titleFont = chatTitle->font();
    titleFont.setBold(true);
    chatTitle->setFont(titleFont);

    topLayout->addWidget(backBtn);
    topLayout->addWidget(chatTitle);
    topLayout->addStretch();

    connect(backBtn, &QPushButton::clicked, this, &MainWindow::switchToChats);

    // Область сообщений
    QScrollArea *scrollArea = new QScrollArea(chatPage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setObjectName("ChatScrollArea");
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    QWidget *messageArea = new QWidget();
    messageArea->setObjectName("MessageArea");
    messageArea->setStyleSheet("background: transparent;");

    QVBoxLayout *scrollLayout = new QVBoxLayout(messageArea);
    scrollLayout->setContentsMargins(10, 10, 10, 10);
    scrollLayout->setSpacing(8);

    messageLayout = scrollLayout;
    scrollArea->setWidget(messageArea);

    // Восстанавливаем историю сообщений
    for (const ChatMessage &msg : chatHistory) {
        QWidget *msgWrapper = createMessageBubble(msg.text, msg.isOutgoing, msg.sender);
        messageLayout->addWidget(msgWrapper);
    }

    messageLayout->addStretch();

    // Нижняя панель
    QWidget *chatBottomBar = new QWidget(chatPage);
    chatBottomBar->setObjectName("ChatBottomBar");
    chatBottomBar->setFixedHeight(70);

    QHBoxLayout *bottomLayout = new QHBoxLayout(chatBottomBar);
    bottomLayout->setContentsMargins(10, 10, 10, 10);
    bottomLayout->setSpacing(10);

    messageEdit = new QLineEdit(chatBottomBar);
    messageEdit->setPlaceholderText("Type a message...");
    messageEdit->setObjectName("MessageInput");
    messageEdit->setMinimumHeight(35);

    QPushButton *sendBtn = new QPushButton(chatBottomBar);
    sendBtn->setObjectName("SendButton");
    sendBtn->setText("Send");
    sendBtn->setFixedSize(80, 35);

    bottomLayout->addWidget(messageEdit);
    bottomLayout->addWidget(sendBtn);

    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(messageEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    mainLayout->addWidget(chatTopBar);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(chatBottomBar);

    stack->addWidget(chatPage);
    stack->setCurrentWidget(chatPage);

    // Прокрутка вниз
    QTimer::singleShot(100, scrollArea->verticalScrollBar(), [scrollArea]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    });
}

QWidget* MainWindow::createMessageBubble(const QString &text, bool isOutgoing, const QString &sender)
{
    QWidget *alignWrapper = new QWidget();
    alignWrapper->setAttribute(Qt::WA_StyledBackground, false);

    QHBoxLayout *alignLayout = new QHBoxLayout(alignWrapper);
    alignLayout->setContentsMargins(10, 4, 10, 4);
    alignLayout->setSpacing(0);

    QWidget *bubbleWidget = new QWidget();

    if (isOutgoing) {
        // Исходящие сообщения - синие
        bubbleWidget->setStyleSheet(
            "background-color: #0084ff;"
            "color: white;"
            "border-radius: 18px;"
            "padding: 8px 12px;"
            "font-size: 14px;"
            );
    } else {
        // Входящие сообщения - серые
        bubbleWidget->setStyleSheet(
            "background-color: #f1f0f0;"
            "color: black;"
            "border-radius: 18px;"
            "padding: 8px 12px;"
            "font-size: 14px;"
            );
    }

    bubbleWidget->setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    bubbleLayout->setSpacing(2);

    // Показываем отправителя для входящих сообщений
    if (!isOutgoing && !sender.isEmpty() && sender != networkManager->getUserName()) {
        QLabel *senderLabel = new QLabel(sender, bubbleWidget);
        senderLabel->setStyleSheet(
            "color: #666;"
            "font-size: 12px;"
            "font-weight: bold;"
            "padding: 0px;"
            "margin: 0px;"
            );
        bubbleLayout->addWidget(senderLabel);
    }

    QLabel *msgLabel = new QLabel(text, bubbleWidget);
    msgLabel->setWordWrap(true);
    msgLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgLabel->setMaximumWidth(350);
    msgLabel->setStyleSheet("background: transparent; border: none; margin: 0px; padding: 0px;");
    bubbleLayout->addWidget(msgLabel);

    // Выравнивание
    if (isOutgoing) {
        alignLayout->addStretch();
        alignLayout->addWidget(bubbleWidget);
        alignLayout->setAlignment(bubbleWidget, Qt::AlignRight);
    } else {
        alignLayout->addWidget(bubbleWidget);
        alignLayout->addStretch();
        alignLayout->setAlignment(bubbleWidget, Qt::AlignLeft);
    }

    return alignWrapper;
}

void MainWindow::sendMessage()
{
    if (!messageEdit || !messageLayout) return;
    const QString text = messageEdit->text().trimmed();
    if (text.isEmpty()) return;

    qDebug() << "Sending message:" << text;

    // Отправляем сообщение через сетевой менеджер
    if (networkManager && networkManager->isConnected()) {
        networkManager->sendMessage(text);
    } else {
        QMessageBox::warning(this, "Ошибка", "Нет подключения к серверу");
        return;
    }

    // Локальное отображение отправленного сообщения (исходящее)
    QWidget *msgWrapper = createMessageBubble(text, true, networkManager->getUserName());
    int count = messageLayout->count();
    if (count > 0) {
        messageLayout->insertWidget(count - 1, msgWrapper);
    } else {
        messageLayout->addWidget(msgWrapper);
    }

    animateMessage(msgWrapper);

    chatHistory.append({text, true, networkManager->getUserName()});
    messageEdit->clear();
    chatActivity["Group Chat"] = QDateTime::currentDateTime();
    refreshChatList();

    // Прокрутка к новому сообщению
    QScrollArea *scrollArea = chatPage->findChild<QScrollArea*>("ChatScrollArea");
    if (scrollArea) {
        QTimer::singleShot(100, scrollArea->verticalScrollBar(), [scrollArea]() {
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
        });
    }
}

void MainWindow::receiveMessage(const QString &text, bool isOutgoing, const QString &sender)
{
    if (!messageLayout) return;

    qDebug() << "Receiving message in UI - Text:" << text << "Outgoing:" << isOutgoing << "Sender:" << sender;

    // Для входящих сообщений используем переданного отправителя
    QString displaySender = sender;
    if (isOutgoing) {
        displaySender = networkManager->getUserName();
    }

    QWidget *msgWrapper = createMessageBubble(text, isOutgoing, displaySender);
    int count = messageLayout->count();
    if (count > 0) {
        messageLayout->insertWidget(count - 1, msgWrapper);
    } else {
        messageLayout->addWidget(msgWrapper);
    }

    animateMessage(msgWrapper);

    chatHistory.append({text, isOutgoing, displaySender});

    // Прокрутка к новому сообщению
    QScrollArea *scrollArea = chatPage->findChild<QScrollArea*>("ChatScrollArea");
    if (scrollArea) {
        QTimer::singleShot(100, scrollArea->verticalScrollBar(), [scrollArea]() {
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
        });
    }
}

void MainWindow::animateMessage(QWidget *target)
{
    auto *opacity = new QGraphicsOpacityEffect(target);
    target->setGraphicsEffect(opacity);
    opacity->setOpacity(0.0);

    QPropertyAnimation *fadeIn = new QPropertyAnimation(opacity, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // Удаляем эффект после завершения
    connect(fadeIn, &QPropertyAnimation::finished, this, [=]() {
        target->setGraphicsEffect(nullptr);
    });

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::switchToChats()
{
    stack->setCurrentIndex(0);
    refreshChatList();

    if (chatPage) {
        int idx = stack->indexOf(chatPage);
        if (idx != -1) {
            QWidget *toRemove = stack->widget(idx);
            stack->removeWidget(toRemove);
            toRemove->deleteLater();
        }
        chatPage = nullptr;
        messageLayout = nullptr;
        messageEdit = nullptr;
    }
}

void MainWindow::switchToSettings()
{
    stack->setCurrentIndex(1);
}

void MainWindow::switchToProfile()
{
    stack->setCurrentIndex(2);
}

void MainWindow::chooseChatWallpaper()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Выберите обои для чата", "",
        "Изображения (*.png *.jpg *.jpeg *.bmp)"
        );
    if (filePath.isEmpty()) return;

    QImage img;
    if (!img.load(filePath)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    selectedWallpaperPath = filePath;

    if (chatPage && chatBackgroundLabel) {
        QPixmap pm = QPixmap::fromImage(img);
        QPixmap scaled = pm.scaled(chatPage->size(),
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
        chatBackgroundLabel->setPixmap(scaled);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (chatBackgroundLabel && chatPage) {
        chatBackgroundLabel->resize(chatPage->size());
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == chatPage && event->type() == QEvent::Resize) {
        if (chatBackgroundLabel) {
            chatBackgroundLabel->resize(chatPage->size());

            if (!selectedWallpaperPath.isEmpty()) {
                QImage img;
                if (img.load(selectedWallpaperPath)) {
                    QPixmap pm = QPixmap::fromImage(img);
                    QPixmap scaled = pm.scaled(chatPage->size(),
                                               Qt::KeepAspectRatioByExpanding,
                                               Qt::SmoothTransformation);
                    chatBackgroundLabel->setPixmap(scaled);
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
