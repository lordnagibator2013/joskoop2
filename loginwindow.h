#pragma once

#include <QWidget>

class LoginForm;
class NetworkManager;
class LoadingDialog;

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void handleLogin(const QString &username, const QString &password);
    void handleCreate(const QString &username, const QString &password);
    void onConnectionStatusChanged(bool connected);
    void onNetworkError(const QString &error);

private:
    LoginForm *form;
    NetworkManager *networkManager;
    LoadingDialog *loadingDialog;
};
