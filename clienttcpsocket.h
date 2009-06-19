#ifndef CLIENTTCPSOCKET_H
#define CLIENTTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>

class ClientTcpSocket : public QObject
{
    Q_OBJECT
public:
    ClientTcpSocket(QObject* parent);
    void connectToHost(QString host, quint16 port);
    void close();
    bool isOpen()
    {
        return socket->isOpen();
    }
private:
    QTcpSocket* socket;
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
public slots:
    void slot_write(QByteArray);
};

#endif // CLIENTTCPSOCKET_H
