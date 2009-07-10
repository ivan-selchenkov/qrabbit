#ifndef HUBTCPSOCKET_H
#define HUBTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class HubTcpSocket : public QObject
{
    Q_OBJECT
public:
    HubTcpSocket(QObject* parent);
    ~HubTcpSocket();
    void connectToHost(QString host, quint16 port);
    void close();
    bool isOpen()
    {
        return socket->isOpen();
    }
private:
    QTcpSocket* socket;
    QByteArray buffer;

    void splitBuffer();
signals:
    void signal_command_received(QByteArray);
private slots:
    void slot_ready_read();
    void error(QAbstractSocket::SocketError);
public slots:
    void slot_write(QByteArray);
};

#endif // HUBTCPSOCKET_H
