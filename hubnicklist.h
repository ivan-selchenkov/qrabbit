#ifndef HUBNICKLIST_H
#define HUBNICKLIST_H
#include <QObject>
#include <QTimer>
#include "hubconnection.h"

struct UserInfo;

class HubConnection;

class HubNickList: public QObject
{
    Q_OBJECT
public:
    HubNickList(QString username, HubConnection* parent = 0);
    QList<UserInfo> list;
    static bool LessThan(const UserInfo &s1, const UserInfo &s2);
public slots:
    void slotMyInfo(QString data);
    void slotQuitMessage(QString);
private slots:
    void slotSortAndUpdate();
signals:
    void signalListChanged();
    void signalListAboutToBeChanged();
private:
    QTimer *timer;
    QMutex m_mutex;
    HubConnection* hub;
    QString m_username;
    bool isMy;
};

struct UserInfo
{
    QString username;   // имя пользователя
    QString description; // описание
    QString client;
    QString mode;
    QString email;
    QString connection;
    QChar status;

    quint64 sharesize; // размер шары

    bool operator== ( const UserInfo & other ) const
    {
        return (this->username == other.username);
    }
    bool operator== ( const QString & other ) const
    {
        return (this->username == other);
    }
    UserInfo& operator= (const UserInfo & other)
    {
        username = other.username;   // имя пользователя
        description = other.description; // описание
        client = other.client;
        mode = other.mode;
        email = other.email;
        connection = other.connection;
        status = other.status;
        sharesize = other.sharesize;
        return *this;
    }
    QString toString(){
        return QString("User: %1, Desc: %2, Client: %3, Email: %4, Con: %5, Share: %6").arg(username).arg(description).arg(client).arg(email).arg(connection).arg(sharesize);
    }
};
#endif // HUBNICKLIST_H
