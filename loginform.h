#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginForm : public QWidget {
    Q_OBJECT
public:
    explicit LoginForm(QWidget *parent = nullptr);

signals:
    void loginRequested(const QString &username, const QString &password);
    void createProfileRequested(const QString &username, const QString &password);

public slots:
    void showError(const QString &message, QLineEdit *field = nullptr);
    void clearErrors();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLabel *errorLabel;

    bool validateInput(QString &username, QString &password);
    void setupUI();

private slots:
    void handleLogin();
    void handleCreate();
};
