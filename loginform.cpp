#include "loginform.h"
#include <QVBoxLayout>

LoginForm::LoginForm(QWidget *parent)
    : QWidget(parent) {
    setupUI();
}

void LoginForm::setupUI() {
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Username");

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QPushButton *loginBtn = new QPushButton("Login", this);
    loginBtn->setObjectName("LoginButton");
    QPushButton *createBtn = new QPushButton("Create Profile", this);
    createBtn->setObjectName("CreateProfileButton");
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(errorLabel);
    layout->addWidget(loginBtn);
    layout->addWidget(createBtn);
    setLayout(layout);

    connect(loginBtn, &QPushButton::clicked, this, &LoginForm::handleLogin);
    connect(createBtn, &QPushButton::clicked, this, &LoginForm::handleCreate);
}

bool LoginForm::validateInput(QString &username, QString &password) {
    clearErrors();
    username = usernameEdit->text();
    password = passwordEdit->text();

    QRegExp userRx("^[A-Za-z0-9_]{1,20}$");
    QRegExp passRx("^[A-Za-z0-9.]{1,20}$");

    if (!userRx.exactMatch(username)) {
        showError("Invalid username", usernameEdit);
        return false;
    }
    if (!passRx.exactMatch(password)) {
        showError("Invalid password", passwordEdit);
        return false;
    }
    return true;
}

void LoginForm::handleLogin() {
    QString user, pass;
    if (validateInput(user, pass)) {
        emit loginRequested(user, pass);
    }
}

void LoginForm::handleCreate() {
    QString user, pass;
    if (validateInput(user, pass)) {
        emit createProfileRequested(user, pass);
    }
}

void LoginForm::showError(const QString &message, QLineEdit *field) {
    if (field) field->setStyleSheet("border: 2px solid red");
    errorLabel->setText(message);
}

void LoginForm::clearErrors() {
    usernameEdit->setStyleSheet("");
    passwordEdit->setStyleSheet("");
    errorLabel->clear();
}
