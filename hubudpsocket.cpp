#include "hubudpsocket.h"

HubUdpSocket::HubUdpSocket(QObject* parent): QObject(parent)
{
    socket = new QUdpSocket(parent);
}
void HubUdpSocket::slot_write(QByteArray data, QString host, quint16 port)
{
    m_write.lock();
    socket->writeDatagram(data, QHostAddress(host), port);
    m_write.unlock();
    qDebug()<<"[SEND UDP" << host << ":"<< port <<"] "+ data;
}
