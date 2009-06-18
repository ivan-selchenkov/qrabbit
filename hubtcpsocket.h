#ifndef HUBTCPSOCKET_H
#define HUBTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>

class HubTcpSocket : public QObject
{
    Q_OBJECT
public:
    HubTcpSocket(QObject* parent);
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
private slots:
    void slot_ready_read();
    void slot_split_buffer();
public slots:
    void slot_write(QByteArray);
};

#endif // HUBTCPSOCKET_H
