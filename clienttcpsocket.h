#ifndef CLIENTTCPSOCKET_H
#define CLIENTTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMutex>
#include <QTimer>

class ClientTcpSocket : public QObject
{
    Q_OBJECT
public:
    ClientTcpSocket(QObject* parent, bool);
    void connectToHost(QString host, quint16 port);
    void close();
    bool isOpen()
    {
        return socket->isOpen();
    }
    bool open(QString&, quint16 &);
private:
    QTcpSocket* socket;
    QTcpServer* server;
    bool isSocket;

    QMutex m_mutex;
    QMutex signal_mutex;
    QMutex write_mutex;

    QByteArray buffer;

    QString m_host;
    quint16 m_port;
signals:
    void signal_command_received(QByteArray);
    void signal_connected();
    void signal_disconnected();
private slots:
    void slot_ready_read();
    void slot_split_buffer();
    void slot_connected();
    void slot_disconnected();
    void slot_new_connection();
    void displayError(QAbstractSocket::SocketError socketError);
public slots:
    void slot_write(QByteArray);
};

#endif // CLIENTTCPSOCKET_H
