#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QWidget *createChatsPage();
    QWidget *createSettingsPage();
    QWidget *createProfilePage();

    QStackedWidget *stack;
    QPushButton *chatsButton;
    QPushButton *settingsButton;
    QPushButton *profileButton;

private slots:
    void switchToChats();
    void switchToSettings();
    void switchToProfile();
};
