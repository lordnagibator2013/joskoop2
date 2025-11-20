#pragma once

#include <QWidget>
#include "loginform.h"

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

private slots:
    void handleLogin(const QString &username, const QString &password);
    void handleCreate(const QString &username, const QString &password);
};
