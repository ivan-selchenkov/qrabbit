#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Crazy Rabbit");
    QCoreApplication::setOrganizationDomain("crazyrabbit.com");
    QCoreApplication::setApplicationName("QRabbitDC++");


    MainWindow w;
    w.show();
    return a.exec();
}
