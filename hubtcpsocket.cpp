#include "hubtcpsocket.h"
#include <QHostAddress>

HubTcpSocket::HubTcpSocket(QObject* parent): QObject(parent)
{
    socket = new QTcpSocket(parent);

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error(QAbstractSocket::SocketError)));

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(slot_ready_read()));
}
HubTcpSocket::~HubTcpSocket()
{
    socket->close();
}

void HubTcpSocket::error(QAbstractSocket::SocketError socketError)
{
    switch(socketError)
    {
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "The connection was refused by the peer (or timed out).";
            break;
        default:
            qDebug() << "socket error" << socketError;
            break;
    }
}

void HubTcpSocket::slot_ready_read()
{
    QByteArray b;
    forever {
        if(socket->bytesAvailable() <= 0) // data is empty - exiting
            break;

        b = socket->readAll();
        buffer.append(b);
        splitBuffer();
    }
}
void HubTcpSocket::splitBuffer()
{
    QList<QByteArray> list_array;
    QString str;

    list_array = buffer.split('|');
    if(list_array.size() > 1)
    {
        // Значит есть новые строки
        if(buffer.at(buffer.size()-1) != '|') // Если строка не кончалась на | то последняя часть обратно в буфер
        {
            buffer = list_array.last();
            list_array.removeLast();
        }
        else
        {
            buffer.clear();
        }
    }
    foreach(QByteArray ba, list_array)
    {
        if(ba.size() > 0)
            emit signal_command_received(ba);
    }
}
void HubTcpSocket::connectToHost(QString host, quint16 port)
{
    socket->connectToHost(host, port);
}
void HubTcpSocket::close()
{
    socket->close();
}
void HubTcpSocket::slot_write(QByteArray data)
{
    qDebug() << "{TCP HUB OUT}" << data;
    socket->write(data);
}
