#include "clientconnection.h"
#include "hubconnection.cpp"
#include <QStringList>
#include <QTimer>
#include <stdlib.h>

ClientConnection::ClientConnection(QObject* parent, QString user, QString addr, QByteArray dCommand): QObject(parent)
{
    username = user;
    QStringList split = addr.split(":");
    if(split.size() < 2) return;
    host = split.at(0);
    QString s_port = split.at(1);
    s_port = s_port.remove(QRegExp("[A-Za-z]"));
    port = s_port.toUInt();

    delayCommand = dCommand;

    clienttcpsocket = new ClientTcpSocket(this);
    connect(clienttcpsocket, SIGNAL(signal_command_received(QByteArray)), this, SLOT(slot_command_received(QByteArray)));
    connect(this, SIGNAL(signal_tcp_write(QByteArray)), clienttcpsocket, SLOT(slot_write(QByteArray)));
    connect(clienttcpsocket, SIGNAL(signal_connected()), this, SLOT(slot_connected()));
    clienttcpsocket->connectToHost(host, port);

    m_isExtended = false;
}
ClientConnection::~ClientConnection()
{
    clienttcpsocket->close();
    delete clienttcpsocket;
}
void ClientConnection::slot_connected()
{
    if(!delayCommand.isEmpty())
        emit slot_command_received(delayCommand);
}
void ClientConnection::slot_command_received(QByteArray data)
{
    QList<QByteArray> list;
    QByteArray response;
    QByteArray lock;

    qDebug() << "{TCP CLIENT IN}" << data;

    if(data.at(0) == '$')
    {
        list = data.split(' '); // Делим строку на части по пробелу
        if(list.size() < 1) return;

        if(list.at(0) == "$ConnectToMe")
        {
            qDebug() << clienttcpsocket->isOpen();
            response.clear();
            response.append(QString("$MyNick %1|").arg(((HubConnection*) this->parent())->userName));
            response.append("$Lock EXTENDEDPROTOCOLABCABCABCABCABCABC Pk=DCPLUSPLUS0.698ABCABC|");
            emit signal_tcp_write(response);
        }
        else if(list.at(0) == "$Lock") { // Клиент послал $Lock
            response.clear();
            if(data.contains("EXTENDEDPROTOCOL"))
            {
                m_isExtended = true;
                response.append("$Supports MiniSlots ADCGet TTHL TTHF XmlBZList |");
                lock = list.at(1);
                lock = lock.remove(0, 16);
            }
            else
                lock = list.at(1);

            // !!!!!!!!! If I'm not want to download !!!!!!!!
            response.append(QString("$Direction Upload 12395|"));

            response.append("$Key ");
            response.append(generateKey(list.at(1)));
            response.append('|');
            emit signal_tcp_write(response);
        }
        else if(list.at(0) == "$ADCGET" && list.size() >= 5)
        {
            if(list.at(1) == "file")
            {
                QString filename = list.at(2);
                quint64 start_pos = list.at(3).toULongLong();
                quint64 size;
                if(list.at(4) != "-1")
                    size = list.at(4).toULongLong();
                else
                    size = 0; // unknown file size

                if(filename == "files.xml.bz2")
                    emit slot_send_filelist(true);
                else if(filename == "files.xml")
                    emit slot_send_filelist(false);
            }
        }
    }
}
void ClientConnection::slot_send_filelist(bool isBZ2)
{
    QString filename;
    if(isBZ2)
        filename = "files.xml.bz2";
    else
        filename = "files.xml";

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) return;

    QByteArray response;
    response.append(QString("$ADCSND file files.xml.bz2 0 %1|").arg(file.size()));
    emit signal_tcp_write(response);

    QByteArray data = file.readAll();
    emit signal_tcp_write(data);
}
QByteArray ClientConnection::generateKey(const QByteArray& lock)
{
    QByteArray key;
    int i;
    int l_size = lock.size();

    for (i = 1; i < l_size; i++)
        key[i] = lock.at(i) ^ lock.at(i-1);

    key[0]=lock[0]^lock[l_size-1]^lock[l_size-2]^5;

    for(i=0; i<l_size; i++) key[i]=( (key[i]<<4) & 240) | ( (key[i]>>4) & 15);

    key.replace((char)0, "/%DCN000%/");
    key.replace((char)5, "/%DCN005%/");
    key.replace((char)36, "/%DCN036%/");
    key.replace((char)96, "/%DCN096%/");
    key.replace((char)124, "/%DCN124%/");
    key.replace((char)126, "/%DCN126%/");

    return key;
}
void ClientConnection::slot_disconnected()
{
}
