#pragma once

#include <QDialog>
#include <QMovie>
#include <QLabel>

class LoadingDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoadingDialog(QWidget *parent = nullptr);
private:
    QLabel *gifLabel;
    QMovie *movie;
};
