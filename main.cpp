#include "mainwindow.h"
#include "alert.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    // Checking if there has been an ALERT flag set up, and if there is, show a big warning
    QFile config("/external_root/boot/flags/ALERT");
    config.open(QIODevice::ReadOnly);
    QTextStream in (&config);
    const QString content = in.readAll();
    string contentstr = content.toStdString();
    config.close();
    if(contentstr.find("true") != std::string::npos) {
        QApplication a(argc, argv);
        alert w;

        w.show();
        return a.exec();
    }
    else {
        QApplication a(argc, argv);
        MainWindow w;

        QApplication::setStyle("windows");
        QFile stylesheetFile(":/resources/eink.qss");
        stylesheetFile.open(QFile::ReadOnly);
        w.setStyleSheet(stylesheetFile.readAll());
        stylesheetFile.close();

        w.show();
        return a.exec();
    }
}
