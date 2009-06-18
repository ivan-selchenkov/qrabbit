#include "hubtcpsocket.h"

HubTcpSocket::HubTcpSocket(QObject* parent): QObject(parent), socket(new QTcpSocket(parent))
{
    //connect(socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(slot_ready_read()));
}
void HubTcpSocket::slot_ready_read()
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
void HubTcpSocket::slot_split_buffer()
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
        emit signal_command_received(ba);
    }
    signal_mutex.unlock();
}
void HubTcpSocket::connectToHost(QString host, quint16 port)
{
    m_host = host;
    m_port = port;

    socket->connectToHost(m_host, port);
}
void HubTcpSocket::close()
{
    socket->close();
}
void HubTcpSocket::slot_write(QByteArray data)
{
    write_mutex.lock();
    socket->write(data);
    write_mutex.unlock();
}
