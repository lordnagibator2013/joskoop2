#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QSpacerItem>
#include <QSizePolicy>

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
    createChatBtn->setFixedSize(40, 40);

    topLayout->addWidget(searchField);
    topLayout->addWidget(createChatBtn);

    // Стек
    stack = new QStackedWidget(this);
    stack->addWidget(createChatsPage());
    stack->addWidget(createSettingsPage());
    stack->addWidget(createProfilePage());

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
    settingsButton = new QPushButton("Settings", bottomBar);
    settingsButton->setObjectName("NavSettings");
    profileButton = new QPushButton("Profile", bottomBar);
    profileButton->setObjectName("NavProfile");

    QList<QPushButton*> buttons = { chatsButton, settingsButton, profileButton };
    for (auto btn : buttons) {
        btn->setFixedSize(100, 30);
        btn->setProperty("class", "NavButton");
        btn->setObjectName("NavButton");
        //btn->setStyleSheet(""); // сбросить встроенные стили
    }

    navLayout->addWidget(chatsButton);
    navLayout->addWidget(settingsButton);
    navLayout->addWidget(profileButton);

    connect(chatsButton, &QPushButton::clicked, this, &MainWindow::switchToChats);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::switchToSettings);
    connect(profileButton, &QPushButton::clicked, this, &MainWindow::switchToProfile);

    // Сборка
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(stack);
    mainLayout->addWidget(bottomBar);
    setCentralWidget(central);
}

QWidget* MainWindow::createChatsPage() {
    QWidget *page = new QWidget(this);
    QLabel *label = new QLabel("Chats will appear here", page);
    label->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(label);
    return page;
}

QWidget* MainWindow::createSettingsPage() {
    QWidget *page = new QWidget(this);
    QLabel *label = new QLabel("Settings", page);
    label->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(label);
    return page;
}

QWidget* MainWindow::createProfilePage() {
    QWidget *page = new QWidget(this);
    QLabel *label = new QLabel("Profile", page);
    label->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(label);
    return page;
}

void MainWindow::switchToChats() {
    stack->setCurrentIndex(0);
}

void MainWindow::switchToSettings() {
    stack->setCurrentIndex(1);
}

void MainWindow::switchToProfile() {
    stack->setCurrentIndex(2);
}
