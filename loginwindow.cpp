#include "loginwindow.h"
#include "loginform.h"
#include "loadingdialog.h"
#include "mainwindow.h"
#include "networkmanager.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
    form(new LoginForm(this)),
    networkManager(new NetworkManager(this)),
    loadingDialog(nullptr)
{
    setWindowTitle("Messenger Login");
    resize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(form);
    setLayout(layout);

    connect(form, &LoginForm::loginRequested, this, &LoginWindow::handleLogin);
    connect(form, &LoginForm::createProfileRequested, this, &LoginWindow::handleCreate);
    connect(networkManager, &NetworkManager::connectionStatusChanged,
            this, &LoginWindow::onConnectionStatusChanged);
    connect(networkManager, &NetworkManager::errorOccurred,
            this, &LoginWindow::onNetworkError);
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::handleLogin(const QString &username, const QString &password)
{
    Q_UNUSED(password)

    loadingDialog = new LoadingDialog(this);
    loadingDialog->show();

    // Пытаемся подключиться к серверу
    networkManager->connectToServer("localhost", 8888, username);

    // Таймер на случай таймаута подключения
    QTimer::singleShot(5000, this, [this]() {
        if (!networkManager->isConnected() && loadingDialog) {
            loadingDialog->close();
            loadingDialog->deleteLater();
            loadingDialog = nullptr;
            QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу");
        }
    });
}

void LoginWindow::handleCreate(const QString &username, const QString &password)
{
    // Пока создание профиля работает так же как логин
    handleLogin(username, password);
}

void LoginWindow::onConnectionStatusChanged(bool connected)
{
    if (connected) {
        if (loadingDialog) {
            loadingDialog->close();
            loadingDialog->deleteLater();
            loadingDialog = nullptr;
        }

        MainWindow *mainWin = new MainWindow(networkManager);  // Исправлено: передаем networkManager
        mainWin->show();
        this->close();
    }
}

void LoginWindow::onNetworkError(const QString &error)
{
    if (loadingDialog) {
        loadingDialog->close();
        loadingDialog->deleteLater();
        loadingDialog = nullptr;
    }

    QMessageBox::warning(this, "Ошибка сети", error);
}
