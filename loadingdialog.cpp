#include "loadingdialog.h"

LoadingDialog::LoadingDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setFixedSize(200, 200);

    gifLabel = new QLabel(this);
    gifLabel->setAlignment(Qt::AlignCenter);
    gifLabel->setGeometry(0, 0, 200, 200);

    movie = new QMovie(":/loading.gif");
    gifLabel->setMovie(movie);
    movie->start();
}
