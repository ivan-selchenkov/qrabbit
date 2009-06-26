#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>
#include "clienttcpsocket.h"

class ClientConnection: public QObject
{
    Q_OBJECT
public:
    ClientConnection(QObject* parent, QString username, QString address, QByteArray delayCommand);
    ~ClientConnection();
private:
    ClientTcpSocket* clienttcpsocket;

    QString username;
    QString host;
    quint16 port;

    QByteArray delayCommand;

    bool m_isExtended;

    QByteArray generateKey(const QByteArray&);

private slots:
    void slot_connected();
    void slot_disconnected();
    void slot_send_filelist(bool isBZ2, bool isList);
public slots:
    void slot_command_received(QByteArray);
signals:
    void signal_tcp_write(QByteArray);
};

#endif // CLIENTCONNECTION_H
