#include "nicklistthreadcontrol.h"
#include <QtCore>

NicklistThreadControl::NicklistThreadControl(QObject* parent, QString user): QThread(parent), username(user)
{
}
void NicklistThreadControl::run()
{
    nicklist = new HubNickList(username);

    connect(this, SIGNAL(signal_myinfo(QString)),
            nicklist, SLOT(slot_myinfo(QString)));
    connect(this, SIGNAL(signal_quit(QString)),
            nicklist, SLOT(slot_quit(QString)));

    connect(nicklist, SIGNAL(signal_list_changed()),
            this, SIGNAL(signal_list_changed()));

    connect(nicklist, SIGNAL(signal_list_about_to_be_changed()),
            this, SIGNAL(signal_list_about_to_be_changed()));

    exec();
    delete nicklist;
    qDebug() << "NicklistThreadControl::~run()";
}
int NicklistThreadControl::size()
{
    return nicklist->size();
}
UserInfo NicklistThreadControl::at(int i)
{
    return nicklist->at(i);
}
