#ifndef HUBUDPSOCKET_H
#define HUBUDPSOCKET_H

#include <QObject>
#include <QMutex>
#include <QUdpSocket>

class HubUdpSocket: public QObject
{
    Q_OBJECT
public:
    HubUdpSocket(QObject* parent);
private:
    QUdpSocket* socket;
    QMutex m_write;
public slots:
    void slot_write(QByteArray data, QString host, quint16 port);
};

#endif // HUBUDPSOCKET_H
