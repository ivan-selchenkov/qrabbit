#ifndef NICKLISTTHREADCONTROL_H
#define NICKLISTTHREADCONTROL_H

#include <QThread>
#include "hubnicklist.h"

class NicklistThreadControl : public QThread
{
    Q_OBJECT
public:
    NicklistThreadControl(QObject* parent, QString username);

    int size();
    UserInfo at(int);
private:
    QString username;
    HubNickList* nicklist;
protected:
    void run();
signals:
    // income signal for hubnicklist
    void signal_myinfo(QString data);
    void signal_quit(QString);

    // outcome signal for refresh widget
    void signal_list_changed();
    void signal_list_about_to_be_changed();

};

#endif // NICKLISTTHREADCONTROL_H
