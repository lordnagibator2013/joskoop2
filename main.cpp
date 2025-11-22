#include <QApplication>
#include "loginwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "loginwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss"); // если в ресурсах
    if (!styleFile.open(QFile::ReadOnly)) {
        qWarning("Could not open style.qss");
    } else {
        QTextStream stream(&styleFile);
        QString style = stream.readAll();
        app.setStyleSheet(style);
    }

    LoginWindow login;
    login.show();
    return app.exec();
}

