#include "clienttcpsocket.h"

ClientTcpSocket::ClientTcpSocket(QObject* parent): QObject(parent), socket(new QTcpSocket(parent))
{
    connect(socket, SIGNAL(connected()), SLOT(slot_connected()));
    connect(socket, SIGNAL(readyRead()), SLOT(slot_ready_read()));
}
void ClientTcpSocket::slot_connected()
{
    qDebug() << "Client connected:" << socket->peerName() << socket->peerPort();
    emit signal_connected();
}
void ClientTcpSocket::slot_ready_read()
{
    QByteArray b;
    forever {
        if(socket->bytesAvailable() <= 0) // Если ничего нет, выходим
            break;
        b = socket->readAll();// Читаем всё что есть
        m_mutex.lock();
            buffer.append(b);
        m_mutex.unlock();
        QTimer::singleShot(0, this, SLOT(slot_split_buffer()));
    }
}
void ClientTcpSocket::slot_split_buffer()
{
    QList<QByteArray> list_array;
    QString str;

    m_mutex.lock();
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
    m_mutex.unlock();

    signal_mutex.lock();
    foreach(QByteArray ba, list_array)
    {
        if(ba.size() > 0)
            emit signal_command_received(ba);
    }
    signal_mutex.unlock();
}
void ClientTcpSocket::connectToHost(QString host, quint16 port)
{
    m_host = host;
    m_port = port;

    socket->connectToHost(m_host, port);
}
void ClientTcpSocket::close()
{
    socket->close();
}
void ClientTcpSocket::slot_write(QByteArray data)
{
    qDebug() << "{TCP CLIENT OUT}" <<data;
    write_mutex.lock();
    socket->write(data);
    write_mutex.unlock();
}
void ClientTcpSocket::slot_disconnected()
{
}
