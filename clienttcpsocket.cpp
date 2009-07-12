#include "clienttcpsocket.h"

ClientTcpSocket::ClientTcpSocket(QObject* parent, bool issocket): QObject(parent), isSocket(issocket)
{
    if(isSocket)
    {
        socket = new QTcpSocket(parent);
        connect(socket, SIGNAL(connected()), SLOT(slot_connected()));
        connect(socket, SIGNAL(readyRead()), SLOT(slot_ready_read()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(displayError(QAbstractSocket::SocketError)));
        connect(socket, SIGNAL(disconnected()), SIGNAL(signal_disconnected()));
        connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
    }
    else
    {
        server = new QTcpServer(parent);
        connect(server, SIGNAL(newConnection()), this, SLOT(slot_new_connection()));
    }
}
void ClientTcpSocket::slot_new_connection()
{
    qDebug() << "slot_new_connection()";
    socket = server->nextPendingConnection();
    connect(socket, SIGNAL(connected()), SLOT(slot_connected()));
    connect(socket, SIGNAL(readyRead()), SLOT(slot_ready_read()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(displayError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SIGNAL(signal_disconnected()));
    connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
}
bool ClientTcpSocket::open(QString& host, quint16 & port)
{
    server->setMaxPendingConnections (1);
    if (!server->listen(QHostAddress(host))) {
         qDebug() << "Unable to start the server: " << server->errorString();
        return false;
    }
    else
    {
        port = server->serverPort();
    }
    return true;
}
void ClientTcpSocket::displayError(QAbstractSocket::SocketError socketError)
{            
    qDebug() << "ClientTcpSocket" << "The following error occurred: %1."<<socket->errorString();
}
void ClientTcpSocket::slot_connected()
{
    qDebug() << "Client connected:" << socket->peerName() << socket->peerPort();
    emit signal_connected();
}
void ClientTcpSocket::slot_ready_read()
{
    qDebug() << "slot_ready_read()";
    QByteArray b;
    forever {
        if(socket->bytesAvailable() <= 0) // Если ничего нет, выходим
            break;
        b = socket->readAll();// Читаем всё что есть
//        m_mutex.lock();
        buffer.append(b);
//        m_mutex.unlock();
        if(b.size() > 1)
            slot_split_buffer();
    }
}
void ClientTcpSocket::slot_split_buffer()
{
    QList<QByteArray> list_array;
    QString str;

//    m_mutex.lock();
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
void ClientTcpSocket::connectToHost(QString host, quint16 port)
{
    m_host = host;
    m_port = port;

    socket->connectToHost(m_host, port);
}
void ClientTcpSocket::close()
{
}
void ClientTcpSocket::slot_write(QByteArray data)
{
    if(data.at(0) == '$')
        qDebug() << "{TCP CLIENT OUT}" <<data;
    qint64 i = socket->write(data);
    qDebug() << "Last size: " << i << "bytes";
}
void ClientTcpSocket::slot_disconnected()
{
}
