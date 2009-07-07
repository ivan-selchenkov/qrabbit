#include <QtGui/QApplication>
#include "mainwindow.h"
#include "searchthreadcontrol.h"

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

    qRegisterMetaType<FileInfo>("FileInfo");
    qRegisterMetaType<SearchItem>("SearchItem");

    MainWindow w;
    w.show();
    return a.exec();
}
