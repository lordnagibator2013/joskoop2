#include "loginwindow.h"
#include "loadingdialog.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTimer>
#include "loginform.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle("Messenger Login");
    resize(300, 200);

    LoginForm *form = new LoginForm(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(form);
    setLayout(layout);

    connect(form, &LoginForm::loginRequested, this, &LoginWindow::handleLogin);
    connect(form, &LoginForm::createProfileRequested, this, &LoginWindow::handleCreate);
}

void LoginWindow::handleLogin(const QString &, const QString &) {
    LoadingDialog *loader = new LoadingDialog(this);
    loader->show();

    QTimer::singleShot(2000, this, [=]() {
        loader->close();
        MainWindow *mainWin = new MainWindow();
        mainWin->show();
        this->close();
    });
}

void LoginWindow::handleCreate(const QString &user, const QString &pass) {
    handleLogin(user, pass); // пока одинаково
}
