#ifndef HUBNICKLIST_H
#define HUBNICKLIST_H
#include <QObject>
#include <QTimer>
#include <QMutex>
#include "userinfo.h"

class HubNickList: public QObject
{
    Q_OBJECT
public:
    HubNickList(QString username);
    QList<UserInfo> list;
    static bool LessThan(const UserInfo &s1, const UserInfo &s2);

    int size();
    UserInfo at(int i);
public slots:
    void slot_myinfo(QString data);
    void slot_quit(QString);
signals:
    void signal_list_changed();
    void signal_list_about_to_be_changed();
private:
    QTimer *timer;
    QMutex m_mutex;
    //HubConnection* hub;
    QString m_username;
    bool isMy;

    void sortAndUpdate();
};

#endif // HUBNICKLIST_H
