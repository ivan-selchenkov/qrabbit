#ifndef HUBTHREADCONTROL_H
#define HUBTHREADCONTROL_H

#include <QThread>
#include "hubconnection.h"
#include "tablemodel.h"
class HubThreadControl : public QThread
{
    Q_OBJECT
public:
    HubThreadControl(QString hostname, quint16 port, QString username, QString password, QString localhost, int slotsNumber, QString email, QString encoding);
    ~HubThreadControl();

    void setSharesize(quint64);
    //void setSlotsNumber(int i);

    TableModel* model;
protected:
    void run();
private:
    HubConnection *hub;

    QString hostname;
    quint16 port;

    QString username;
    QString password;
    QString localhost;
    int slotsNumber;
    QString email;
    QString encoding;

    NicklistThreadControl* nicklistControl;
signals:
    // outcome signals
    void signal_hub_message(QString);
    void signal_search_request(SearchItem);

    // incoming signals for HubConnection
    void signal_sharesize(quint64);
    void signal_search_result(FileInfo,SearchItem);
};

#endif // HUBTHREADCONTROL_H
