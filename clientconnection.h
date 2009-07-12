#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>
#include "clienttcpsocket.h"
#include <QFile>

class ClientConnection: public QObject
{
    Q_OBJECT
public:
    ClientConnection(QObject* parent, QString username, QString address, QByteArray delayCommand, bool isActive);
    ~ClientConnection();

    QString username;
private:
    ClientTcpSocket* clienttcpsocket;

    QString host;
    quint16 port;

    QByteArray delayCommand;

    bool m_isExtended;

    QByteArray generateKey(const QByteArray&);
    void sendFilelist(bool isBZ2, bool isList);
    void sendTthl(QString tth, quint64 start, quint64 len, QString flags);
    void sendFile(QString tth, quint64 start, quint64 len, QString flags);

    QFile uploadFileTTH;
    QString uploadFileName;
    QString uploadTTH;
private slots:
    void slot_connected();
    void slot_disconnected();
public slots:
    void slot_command_received(QByteArray);
signals:
    void signal_tcp_write(QByteArray);
    void signal_hub_tcp_write(QByteArray);
};

#endif // CLIENTCONNECTION_H
