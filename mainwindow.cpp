#include "mainwindow.h"
#include "networkmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QScrollArea>
#include <QScrollBar>
#include <algorithm>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
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
    // Очищаем активные анимации
    for (QAbstractAnimation *anim : std::as_const(activeAnimations)) {
        anim->stop();
        anim->deleteLater();
    }
    activeAnimations.clear();
}

void MainWindow::onNetworkMessageReceived(const QString &sender, const QString &text, bool isOwnMessage)
{
    qDebug() << "Displaying message - Sender:" << sender << "Text:" << text << "IsOwn:" << isOwnMessage;

    // Если чат открыт, отображаем сообщение
    if (chatPage && stack->currentWidget() == chatPage) {
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

    layout->addWidget(chatListContainer);
    return page;
}

QWidget* MainWindow::createSettingsPage() {
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("Settings", page);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QPushButton *wallpaperBtn = new QPushButton("Сменить обои чата", page);
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
    if (!layout) return;

    while (QLayoutItem *child = layout->takeAt(0)) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    QList<QString> sortedChats = chatActivity.keys();
    std::sort(sortedChats.begin(), sortedChats.end(), [&](const QString &a, const QString &b) {
        return chatActivity[a] > chatActivity[b];
    });

    for (const QString &chatName : sortedChats) {
        QPushButton *chatBtn = new QPushButton(chatName, chatListContainer);
        chatBtn->setObjectName("ChatEntry");
        chatBtn->setFixedHeight(70); // Увеличиваем высоту с 40 до 70

        // Устанавливаем иконку для Group Chat
        if (chatName == "Group Chat") {
            chatBtn->setIcon(QIcon("://group_chat_icon.png")); // Замените на ваш путь к иконке
            chatBtn->setIconSize(QSize(100, 120));
        }

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
    topLayout->setContentsMargins(10, 5, 10, 5); // Уменьшаем верхний и нижний отступы
    topLayout->setSpacing(10);

    QPushButton *backBtn = new QPushButton(chatTopBar);
    backBtn->setObjectName("BackButton");
    backBtn->setIcon(QIcon("://back.png"));
    backBtn->setIconSize(QSize(20, 20));
    backBtn->setFixedSize(40, 40);

    QLabel *chatTitle = new QLabel("Group Chat", chatTopBar);
    chatTitle->setObjectName("ChatTitle");
    QFont titleFont = chatTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14); // Увеличим шрифт для лучшего баланса
    chatTitle->setFont(titleFont);

    topLayout->addWidget(backBtn);
    topLayout->addWidget(chatTitle);
    topLayout->addStretch();

    // Устанавливаем выравнивание по центру по вертикали для всех элементов
    topLayout->setAlignment(Qt::AlignCenter);

    connect(backBtn, &QPushButton::clicked, this, &MainWindow::switchToChats);

    // Область сообщений
    QScrollArea *scrollArea = new QScrollArea(chatPage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setObjectName("ChatScrollArea");
    scrollArea->setStyleSheet("QScrollArea#ChatScrollArea { background: transparent; border: none; }");

    QWidget *messageArea = new QWidget();
    messageArea->setObjectName("MessageArea");
    messageArea->setStyleSheet("QWidget#MessageArea { background: transparent; }");

    QVBoxLayout *scrollLayout = new QVBoxLayout(messageArea);
    scrollLayout->setContentsMargins(10, 10, 10, 10);
    scrollLayout->setSpacing(8);
    scrollLayout->setAlignment(Qt::AlignTop);

    messageLayout = scrollLayout;
    scrollArea->setWidget(messageArea);

    // УБРАНО: Восстановление истории сообщений
    // Сообщения будут добавляться через receiveMessage при получении от сервера

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

    // Кнопка смайликов
    QPushButton *emojiButton = new QPushButton(chatBottomBar);
    emojiButton->setObjectName("EmojiButton");
    emojiButton->setIcon(QIcon("://emoji_icon.png")); // Замените на ваш путь к иконке
    emojiButton->setIconSize(QSize(24, 24));
    emojiButton->setFixedSize(40, 40);
    emojiButton->setToolTip("Insert emoji");

    QPushButton *sendBtn = new QPushButton(chatBottomBar);
    sendBtn->setObjectName("SendButton");
    sendBtn->setIcon(QIcon("://send.png"));
    sendBtn->setIconSize(QSize(20, 20));
    sendBtn->setFixedSize(40, 40);

    bottomLayout->addWidget(messageEdit);
     bottomLayout->addWidget(emojiButton);
    bottomLayout->addWidget(sendBtn);

    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(messageEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    mainLayout->addWidget(chatTopBar);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(chatBottomBar);

    stack->addWidget(chatPage);
    stack->setCurrentWidget(chatPage);

    // Восстанавливаем сообщения через receiveMessage для каждого сообщения в истории
    for (const ChatMessage &msg : chatHistory) {
        receiveMessage(msg.text, msg.isOutgoing, msg.sender);
    }

    connect(emojiButton, &QPushButton::clicked, this, &MainWindow::showEmojiPicker);
}

// ✅ ИСПРАВЛЕНО: пузырьки сообщений теперь правильно отображаются
QWidget* MainWindow::createMessageBubble(const QString &text, bool isOutgoing, const QString &sender)
{
    QWidget *alignWrapper = new QWidget();
    alignWrapper->setAttribute(Qt::WA_TranslucentBackground);
    alignWrapper->setObjectName("MessageWrapper");
    alignWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *alignLayout = new QHBoxLayout(alignWrapper);
    alignLayout->setContentsMargins(10, 4, 10, 4);
    alignLayout->setSpacing(0);

    QFrame *bubbleWidget = new QFrame();
    bubbleWidget->setFrameShape(QFrame::NoFrame);
    bubbleWidget->setObjectName("MessageBubble");
    bubbleWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    // Проверяем, является ли сообщение только эмодзи
    bool emojiOnly = isEmojiOnly(text);

    if (emojiOnly) {
        // Стиль для сообщений только с эмодзи - прозрачный фон, большой шрифт
        bubbleWidget->setStyleSheet(
            "QFrame#MessageBubble {"
            "   background: transparent;"
            "   border: none;"
            "   padding: 0px;"
            "   font-size: 32px;"  // Увеличиваем размер шрифта для эмодзи
            "}"
            );
    } else {
        // Обычный стиль для текстовых сообщений
        if (isOutgoing) {
            bubbleWidget->setStyleSheet(
                "QFrame#MessageBubble {"
                "   background-color: #0084ff;"
                "   color: white;"
                "   border-radius: 17px;"
                "   padding: 8px 12px;"
                "   font-size: 14px;"
                "   border: none;"
                "}"
                );
        } else {
            bubbleWidget->setStyleSheet(
                "QFrame#MessageBubble {"
                "   background-color: #bbc9b7;"
                "   color: black;"
                "   border-radius: 17px;"
                "   padding: 8px 12px;"
                "   font-size: 14px;"
                "   border: none;"
                "}"
                );
        }
    }

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    bubbleLayout->setSpacing(2);

    // Не показываем отправителя для сообщений только с эмодзи
    if (!isOutgoing && !sender.isEmpty() && sender != networkManager->getUserName() && !emojiOnly) {
        QLabel *senderLabel = new QLabel(sender, bubbleWidget);
        senderLabel->setObjectName("SenderLabel");
        senderLabel->setStyleSheet(
            "QLabel#SenderLabel {"
            "   color: #666;"
            "   font-size: 12px;"
            "   font-weight: bold;"
            "   padding: 0px;"
            "   margin: 0px;"
            "   background: transparent;"
            "}"
            );
        senderLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        bubbleLayout->addWidget(senderLabel);
    }

    QLabel *msgLabel = new QLabel(text, bubbleWidget);
    msgLabel->setObjectName("MessageText");
    msgLabel->setWordWrap(true);
    msgLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    if (emojiOnly) {
        // Для эмодзи убираем ограничения по ширине и увеличиваем шрифт
        msgLabel->setMaximumWidth(1000);
        msgLabel->setStyleSheet(
            "QLabel#MessageText { "
            "   background: transparent; "
            "   border: none; "
            "   margin: 0; "
            "   padding: 0; "
            "   font-size: 32px;"
            "}"
            );
        msgLabel->setAlignment(Qt::AlignCenter);
    } else {
        // Обычные настройки для текстовых сообщений
        msgLabel->setMaximumWidth(350);
        msgLabel->setStyleSheet(
            "QLabel#MessageText { "
            "   background: transparent; "
            "   border: none; "
            "   margin: 0; "
            "   padding: 0; "
            "   color: inherit;"
            "}"
            );
    }

    msgLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    bubbleLayout->addWidget(msgLabel);

    // Для эмодзи-сообщений не устанавливаем минимальную ширину
    if (!emojiOnly) {
        int textWidth = QFontMetrics(msgLabel->font()).horizontalAdvance(text);
        int minWidth = qMin(qMax(textWidth, 50), 350);
        msgLabel->setMinimumWidth(minWidth);
    }

    // Форсируем расчет размеров ДО отображения
    bubbleWidget->adjustSize();
    alignWrapper->adjustSize();

    // Для эмодзи-сообщений не устанавливаем фиксированную высоту
    if (!emojiOnly) {
        int calculatedHeight = bubbleWidget->height();
        alignWrapper->setFixedHeight(calculatedHeight + 8);
    }

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

/*void MainWindow::animateMessage(QWidget *target)
{
    if (!target || !target->parentWidget()) {
        return;
    }

    // Простая анимация без сложных связей
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(target);
    target->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(250);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    // Автоматическое удаление при завершении
    animation->start(QPropertyAnimation::DeleteWhenStopped);

    // Удаляем эффект после анимации
    connect(animation, &QPropertyAnimation::finished, target, [target, effect]() {
        target->setGraphicsEffect(nullptr);
        effect->deleteLater();
    });
}*/

void MainWindow::sendMessage()
{
    if (!messageEdit || !messageLayout) return;
    const QString text = messageEdit->text().trimmed();
    if (text.isEmpty()) return;

    qDebug() << "Sending message:" << text;

    if (networkManager && networkManager->isConnected()) {
        networkManager->sendMessage(text);
    } else {
        QMessageBox::warning(this, "Ошибка", "Нет подключения к серверу");
        return;
    }

    // НЕ добавляем сообщение локально - ждем получения от сервера
    // Это предотвратит дублирование

    chatHistory.append({text, true, networkManager->getUserName()});
    messageEdit->clear();
    chatActivity["Group Chat"] = QDateTime::currentDateTime();
    refreshChatList();
}

void MainWindow::receiveMessage(const QString &text, bool isOutgoing, const QString &sender)
{
    if (!messageLayout) return;

    qDebug() << "Receiving message in UI - Text:" << text << "Outgoing:" << isOutgoing << "Sender:" << sender;

    // Проверяем, нет ли уже такого сообщения в макете
    bool messageAlreadyExists = false;
    for (int i = 0; i < messageLayout->count() - 1; ++i) { // -1 чтобы исключить stretch
        QLayoutItem *item = messageLayout->itemAt(i);
        if (item && item->widget()) {
            QLabel *msgLabel = item->widget()->findChild<QLabel*>("MessageText");
            if (msgLabel && msgLabel->text() == text) {
                messageAlreadyExists = true;
                break;
            }
        }
    }

    if (messageAlreadyExists) {
        qDebug() << "Message already exists in layout, skipping";
        return;
    }

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

    // Добавляем в историю только если это новое сообщение
    bool messageExistsInHistory = false;
    for (const ChatMessage &msg : chatHistory) {
        if (msg.text == text && msg.sender == displaySender && msg.isOutgoing == isOutgoing) {
            messageExistsInHistory = true;
            break;
        }
    }

    if (!messageExistsInHistory) {
        chatHistory.append({text, isOutgoing, displaySender});
    }

    QScrollArea *scrollArea = chatPage->findChild<QScrollArea*>("ChatScrollArea");
    if (scrollArea) {
        QTimer::singleShot(100, scrollArea->verticalScrollBar(), [scrollArea]() {
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
        });
    }
}

void MainWindow::switchToChats()
{
    // ✅ Останавливаем все активные анимации перед удалением чата
    for (QAbstractAnimation *anim : std::as_const(activeAnimations)) {
        if (anim->state() == QAbstractAnimation::Running) {
            anim->stop();
        }
    }
    // Удаляем отложенно — безопасно
    for (QAbstractAnimation *anim : std::as_const(activeAnimations)) {
        anim->deleteLater();
    }
    activeAnimations.clear();

    stack->setCurrentIndex(0);
    refreshChatList();

    if (chatPage) {
        chatPage->removeEventFilter(this);

        int idx = stack->indexOf(chatPage);
        if (idx != -1) {
            QWidget *toRemove = stack->widget(idx);
            stack->removeWidget(toRemove);
            toRemove->setParent(nullptr);
            toRemove->deleteLater();
        }
        chatPage = nullptr;
        messageLayout = nullptr;
        messageEdit = nullptr;
        chatBackgroundLabel = nullptr;

        // Если хотите очищать историю при выходе из чата, раскомментируйте следующую строку:
        // chatHistory.clear();
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

void MainWindow::showEmojiPicker()
{
    if (!messageEdit) return;

    // Создаем диалог для выбора смайликов
    QDialog *emojiDialog = new QDialog(this);
    emojiDialog->setWindowTitle("Select Emoji");
    emojiDialog->setFixedSize(450, 350); // Еще больше увеличили размер окна
    emojiDialog->setModal(true);
    emojiDialog->setStyleSheet("QDialog { background-color: #2c3e50; border-radius: 10px; }");

    QGridLayout *layout = new QGridLayout(emojiDialog);
    layout->setSpacing(8); // Еще больше увеличили расстояние между кнопками
    layout->setContentsMargins(15, 15, 15, 15); // Увеличили отступы от краев

    // Список популярных смайликов
    QStringList emojis = {
        "😀", "😃", "😄", "😁", "😆", "😅", "😂", "🤣",
        "😊", "😇", "🙂", "🙃", "😉", "😌", "😍", "🥰",
        "😘", "😗", "😙", "😚", "😋", "😛", "😝", "😜",
        "🤪", "🤨", "🧐", "🤓", "😎", "🤩", "🥳", "😏",
        "😒", "😞", "😔", "😟", "😕", "🙁", "☹️", "😣",
        "😖", "😫", "😩", "🥺", "😢", "😭", "😤", "😠",
        "😡", "🤬", "🤯", "😳", "🥵", "🥶", "😱", "😨",
        "😰", "😥", "😓", "🤗", "🤔", "🤭", "🤫", "🤥",
        "😶", "😐", "😑", "😬", "🙄", "😯", "😦", "😧",
        "😮", "😲", "🥱", "😴", "🤤", "😪", "😵", "🤐",
        "🥴", "🤢", "🤮", "🤧", "😷", "🤒", "🤕", "🤑",
        "🤠", "😈", "👿", "👹", "👺", "🤡", "💩", "👻",
        "💀", "☠️", "👽", "👾", "🤖", "🎃", "😺", "😸",
        "😹", "😻", "😼", "😽", "🙀", "😿", "😾"
    };

    // Создаем кнопки для каждого смайлика
    int row = 0, col = 0;
    for (const QString &emoji : emojis) {
        QPushButton *emojiBtn = new QPushButton(emoji, emojiDialog);
        emojiBtn->setFixedSize(40, 40); // Еще больше увеличили размер кнопок
        emojiBtn->setFont(QFont("Segoe UI Emoji", 16)); // Еще больше увеличили шрифт

        // Убираем фон и границы у кнопок с эмодзи
        emojiBtn->setStyleSheet(
            "QPushButton {"
            "   background: transparent;"
            "   border: none;"
            "   border-radius: 5px;"
            "   padding: 0px;"
            "   margin: 0px;"
            "}"
            "QPushButton:hover {"
            "   background: rgba(255,255,255,0.1);"
            "}"
            "QPushButton:pressed {"
            "   background: rgba(255,255,255,0.2);"
            "}"
            );

        connect(emojiBtn, &QPushButton::clicked, this, [this, emoji, emojiDialog]() {
            if (messageEdit) {
                messageEdit->insert(emoji);
            }
            emojiDialog->close();
        });

        layout->addWidget(emojiBtn, row, col);
        col++;
        if (col >= 9) { // Немного уменьшили количество столбцов до 9
            col = 0;
            row++;
        }
    }

    // Добавляем кнопку закрытия
    QPushButton *closeBtn = new QPushButton("Close", emojiDialog);
    closeBtn->setFixedHeight(35);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #34495e;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3d566e;"
        "}"
        );

    connect(closeBtn, &QPushButton::clicked, emojiDialog, &QDialog::close);

    layout->addWidget(closeBtn, row + 1, 0, 1, 9, Qt::AlignCenter); // Размещаем кнопку закрытия

    emojiDialog->exec();
    emojiDialog->deleteLater();
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

bool MainWindow::isEmojiOnly(const QString &text)
{
    if (text.isEmpty()) return false;

    // Регулярное выражение для определения эмодзи
    // Оно покрывает большинство эмодзи, включая составные
    QRegularExpression emojiRegex(
        "^[\\x{1F600}-\\x{1F64F}"      // Emoticons
        "\\x{1F300}-\\x{1F5FF}"        // Misc Symbols and Pictographs
        "\\x{1F680}-\\x{1F6FF}"        // Transport & Map
        "\\x{1F1E0}-\\x{1F1FF}"        // Flags (iOS)
        "\\x{2600}-\\x{26FF}"          // Misc symbols
        "\\x{2700}-\\x{27BF}"          // Dingbats
        "\\x{FE00}-\\x{FE0F}"          // Variation Selectors
        "\\x{1F900}-\\x{1F9FF}"        // Supplemental Symbols and Pictographs
        "\\x{1F018}-\\x{1F270}"        // Various symbols
        "]*$"
        );
    return emojiRegex.match(text).hasMatch() && text.length() <= 4;
}
