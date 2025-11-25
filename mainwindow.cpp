#include "mainwindow.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Messenger");
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
    createChatBtn->setIcon(QIcon("://plus.png"));
    createChatBtn->setIconSize(QSize(20, 20));
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

    // Создаём кнопки с уникальными objectName
    chatsButton = new QPushButton("Chats", bottomBar);
    chatsButton->setObjectName("NavChats");
    chatsButton->setFixedSize(100, 30);

    settingsButton = new QPushButton("Settings", bottomBar);
    settingsButton->setObjectName("NavSettings");
    settingsButton->setFixedSize(100, 30);

    profileButton = new QPushButton("Profile", bottomBar);
    profileButton->setObjectName("NavProfile");
    profileButton->setFixedSize(100, 30);

    // Добавляем кнопки в нижнюю панель
    navLayout->addWidget(chatsButton);
    navLayout->addWidget(settingsButton);
    navLayout->addWidget(profileButton);

    // Подключаем сигналы
    connect(chatsButton,   &QPushButton::clicked, this, &MainWindow::switchToChats);
    connect(settingsButton,&QPushButton::clicked, this, &MainWindow::switchToSettings);
    connect(profileButton, &QPushButton::clicked, this, &MainWindow::switchToProfile);

    // Сборка
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(stack);
    mainLayout->addWidget(bottomBar);
    setCentralWidget(central);

    // Инициализация активности и списка чатов
    chatActivity["Me"] = QDateTime::currentDateTime();
    refreshChatList();
    stack->setCurrentIndex(0);
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
    layout->addWidget(new QLabel("Settings", page));
    return page;
}

QWidget* MainWindow::createProfilePage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel("Profile", page));
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

        if (chatName == "Me") {
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
    chatPage->installEventFilter(this); // для отслеживания изменения размера
    chatPage->setAttribute(Qt::WA_StyledBackground, true);

    // Фоновое изображение (только если выбран путь)
    if (!selectedWallpaperPath.isEmpty()) {
        chatBackgroundLabel = new QLabel(chatPage);
        chatBackgroundLabel->setObjectName("ChatBackground");
        chatBackgroundLabel->setScaledContents(true);
        chatBackgroundLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        chatBackgroundLabel->lower(); // на задний план
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
    backBtn->setIcon(QIcon("://back.png"));
    backBtn->setIconSize(QSize(20, 20));
    backBtn->setFixedSize(40, 40);

    QLabel *chatTitle = new QLabel("Me", chatTopBar);
    chatTitle->setObjectName("ChatTitle");

    topLayout->addWidget(backBtn);
    topLayout->addWidget(chatTitle);
    topLayout->addStretch();

    connect(backBtn, &QPushButton::clicked, this, &MainWindow::switchToChats);

    // Область сообщений
    QScrollArea *scrollArea = new QScrollArea(chatPage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setObjectName("ChatScrollArea");
    scrollArea->setStyleSheet("background: transparent; border: none;");

    QWidget *messageArea = new QWidget();
    messageArea->setObjectName("MessageArea");

    QVBoxLayout *scrollLayout = new QVBoxLayout(messageArea);
    scrollLayout->setContentsMargins(10, 10, 10, 10);
    scrollLayout->setSpacing(10);

    messageLayout = scrollLayout;
    scrollArea->setWidget(messageArea);

    for (const ChatMessage &msg : chatHistory) {
        QWidget *msgWrapper = createMessageBubble(msg.text, msg.isOutgoing);
        messageLayout->addWidget(msgWrapper);
    }

    messageLayout->addStretch();

    // Нижняя панель
    QWidget *chatBottomBar = new QWidget(chatPage);
    chatBottomBar->setObjectName("ChatBottomBar");
    chatBottomBar->setFixedHeight(60);

    QHBoxLayout *bottomLayout = new QHBoxLayout(chatBottomBar);
    bottomLayout->setContentsMargins(10, 10, 10, 10);
    bottomLayout->setSpacing(10);

    messageEdit = new QLineEdit(chatBottomBar);
    messageEdit->setPlaceholderText("Type a message...");
    messageEdit->setObjectName("MessageInput");

    QPushButton *sendBtn = new QPushButton(chatBottomBar);
    sendBtn->setObjectName("SendButton");
    sendBtn->setIcon(QIcon("://send.png"));
    sendBtn->setIconSize(QSize(20, 20));
    sendBtn->setFixedSize(40, 40);

    bottomLayout->addWidget(messageEdit);
    bottomLayout->addWidget(sendBtn);

    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendMessage);

    // Сборка
    mainLayout->addWidget(chatTopBar);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(chatBottomBar);

    stack->addWidget(chatPage);
    stack->setCurrentWidget(chatPage);

    // Прокрутка вниз
    QTimer::singleShot(0, scrollArea->verticalScrollBar(), [scrollArea]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    });

    // Первое сообщение
    if (!hasReceivedInitialMessage) {
        receiveMessage("Пошла нахуй мама ебаная");
        hasReceivedInitialMessage = true;
    }
}

QWidget* MainWindow::createMessageBubble(const QString &text, bool isOutgoing) {
    // Контейнер для выравнивания (без фона)
    QWidget *alignWrapper = new QWidget();
    alignWrapper->setAttribute(Qt::WA_StyledBackground, false);


    QHBoxLayout *alignLayout = new QHBoxLayout(alignWrapper);
    alignLayout->setContentsMargins(10, 6, 10, 6);
    alignLayout->setSpacing(0);

    // Сам пузырёк (фон и рамка рисуются через QSS)
    QWidget *msgWrapper = new QWidget();
    msgWrapper->setObjectName(isOutgoing ? "MessageBubble" : "IncomingBubble");
    if (isOutgoing){
        msgWrapper->setStyleSheet("background-color: #448aff;");
    }
    else{
        msgWrapper->setStyleSheet("background-color: #dfe6e9;");
    }

    msgWrapper->setAttribute(Qt::WA_StyledBackground, true); // критично!

    // Внутренний текст
    QLabel *msgLabel = new QLabel(text, msgWrapper);
    msgLabel->setWordWrap(true);
    msgLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Ограничение ширины
    QFontMetrics fm(msgLabel->font());
    int maxWidth = 400;
    msgLabel->setMaximumWidth(maxWidth);

    // Внутренний layout пузырька (имитируем padding из QSS для корректной вёрстки)
    QVBoxLayout *bubbleLayout = new QVBoxLayout(msgWrapper);
    bubbleLayout->setContentsMargins(14, 8, 14, 8); // соответствует padding в QSS
    bubbleLayout->setSpacing(0);
    bubbleLayout->addWidget(msgLabel);

    // Выравнивание
    if (isOutgoing) {
        alignLayout->addStretch();
        alignLayout->addWidget(msgWrapper);
    } else {
        alignLayout->addWidget(msgWrapper);
        alignLayout->addStretch();
    }

    return alignWrapper;
}

void MainWindow::sendMessage() {
    if (!messageEdit || !messageLayout) return;
    const QString text = messageEdit->text().trimmed();
    if (text.isEmpty()) return;

    QWidget *msgWrapper = createMessageBubble(text, true);
    int count = messageLayout->count();
    if (count > 0) {
        messageLayout->insertWidget(count - 1, msgWrapper);
    } else {
        messageLayout->addWidget(msgWrapper);
    }

    animateMessage(msgWrapper);

    chatHistory.append({text, true});
    messageEdit->clear();
    chatActivity["Me"] = QDateTime::currentDateTime();
    refreshChatList();
}

void MainWindow::receiveMessage(const QString &text) {
    if (!messageLayout) return;

    QWidget *msgWrapper = createMessageBubble(text, false);
    int count = messageLayout->count();
    if (count > 0) {
        messageLayout->insertWidget(count - 1, msgWrapper);
    } else {
        messageLayout->addWidget(msgWrapper);
    }

    animateMessage(msgWrapper);

    chatHistory.append({text, false});
}

void MainWindow::animateMessage(QWidget *target) {
    auto *opacity = new QGraphicsOpacityEffect(target);
    target->setGraphicsEffect(opacity);
    opacity->setOpacity(0.0);

    // Анимация прозрачности
    QPropertyAnimation *fadeIn = new QPropertyAnimation(opacity, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // Анимация прокрутки
    QScrollArea *scrollArea = chatPage->findChild<QScrollArea*>("ChatScrollArea");
    if (scrollArea) {
        QScrollBar *vScroll = scrollArea->verticalScrollBar();
        int start = vScroll->value();
        int end = vScroll->maximum();

        QPropertyAnimation *scrollAnim = new QPropertyAnimation(vScroll, "value");
        scrollAnim->setDuration(300);
        scrollAnim->setStartValue(start);
        scrollAnim->setEndValue(end);
        scrollAnim->setEasingCurve(QEasingCurve::OutCubic);

        scrollAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // Удаляем эффект после завершения
    connect(fadeIn, &QPropertyAnimation::finished, this, [=]() {
        target->setGraphicsEffect(nullptr);
    });

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::switchToChats() {
    // Вернуться на страницу чатов
    stack->setCurrentIndex(0);
    refreshChatList();

    // Очистить динамическую страницу чата
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

void MainWindow::switchToSettings() {
    stack->setCurrentIndex(1);
    QWidget *settingsPage = stack->widget(1);
    if (!settingsPage->findChild<QPushButton*>("WallpaperButton")) {
        QPushButton *wallpaperBtn = new QPushButton("Сменить обои", settingsPage);
        wallpaperBtn->setObjectName("WallpaperButton");
        wallpaperBtn->setFixedHeight(40);

        QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(settingsPage->layout());
        if (!layout) {
            layout = new QVBoxLayout(settingsPage);
            settingsPage->setLayout(layout);
        }

        layout->addWidget(wallpaperBtn);

        connect(wallpaperBtn, &QPushButton::clicked, this, &MainWindow::chooseChatWallpaper);
    }
}

void MainWindow::chooseChatWallpaper() {
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

    // Если чат уже открыт — применим сразу
    if (chatPage && chatBackgroundLabel) {
        QPixmap pm = QPixmap::fromImage(img);
        QPixmap scaled = pm.scaled(chatPage->size(),
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
        chatBackgroundLabel->setPixmap(scaled);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if (chatBackgroundLabel && chatPage) {
        chatBackgroundLabel->resize(chatPage->size());
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
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

void MainWindow::switchToProfile() {
    stack->setCurrentIndex(2);
}
