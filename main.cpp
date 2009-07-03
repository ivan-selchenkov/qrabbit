#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Crazy Rabbit");
    QCoreApplication::setOrganizationDomain("crazyrabbit.com");
    QCoreApplication::setApplicationName("QRabbitDC++");

    QTextCodec *codec = QTextCodec::codecForName("UTF8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);

    MainWindow w;
    w.show();
    return a.exec();
}
