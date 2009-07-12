#include "clientconnection.h"
#include "hubconnection.cpp"
#include <QStringList>
#include <QTimer>
#include <QtSql>
#include <stdlib.h>

ClientConnection::ClientConnection(QObject* parent, QString user, QString addr, QByteArray dCommand, bool isActive):
        QObject(parent), username(user), delayCommand(dCommand)
{
    clienttcpsocket = new ClientTcpSocket(this, isActive);

    connect(clienttcpsocket, SIGNAL(signal_command_received(QByteArray)),
            this, SLOT(slot_command_received(QByteArray)));
    connect(clienttcpsocket, SIGNAL(signal_connected()),
            this, SLOT(slot_connected()));
    connect(clienttcpsocket, SIGNAL(signal_disconnected()), SLOT(deleteLater()));

    connect(clienttcpsocket, SIGNAL(destroyed()), this, SLOT(deleteLater()));

    connect(this, SIGNAL(signal_tcp_write(QByteArray)),
            clienttcpsocket, SLOT(slot_write(QByteArray)));

    connect(this, SIGNAL(signal_hub_tcp_write(QByteArray)), (HubConnection*)parent, SIGNAL(signal_tcp_write(QByteArray)));


    if(isActive)
    {
        QStringList split = addr.split(":");
        if(split.size() < 2) return;
        host = split.at(0);
        QString s_port = split.at(1);
        s_port = s_port.remove(QRegExp("[A-Za-z]"));
        port = s_port.toUInt();
        clienttcpsocket->connectToHost(host, port);
    }
    else
    {
        clienttcpsocket->open(addr, port);
        QByteArray result;
        result.append(QString("$ConnectToMe %1 %2:%3|").arg(user).arg(addr).arg(port));
        signal_hub_tcp_write(result);
    }

    m_isExtended = false;
}
ClientConnection::~ClientConnection()
{
    clienttcpsocket->close();
    delete clienttcpsocket;
    if(uploadFileTTH.isOpen()) uploadFileTTH.close();
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

        if(list.at(0) == "$ConnectToMe" || list.at(0) == "$MyNick" /* if other client passive */ )
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
                lock = list.at(1);
                lock = lock.remove(0, 16);
            }
            else
                lock = list.at(1);

            // !!!!!!!!! If I'm not want to download !!!!!!!!
            response.append("$Supports MiniSlots ADCGet TTHL TTHF XmlBZList |");
            response.append(QString("$Direction Upload 12395|"));
            response.append("$Key ");
            response.append(generateKey(list.at(1)));
            response.append('|');

            emit signal_tcp_write(response);
        }
        else if(list.at(0) == "$ADCGET" && list.size() >= 5)
        {
            if(list.at(1) == "file" && (list.at(2) == "files.xml.bz2" || list.at(2) == "files.xml"))
            {
                QString filename = list.at(2);
                quint64 start_pos = list.at(3).toULongLong();
                quint64 size;
                if(list.at(4) != "-1")
                    size = list.at(4).toULongLong();
                else
                    size = 0; // unknown file size

                if(filename == "files.xml.bz2")
                    sendFilelist(true, false);
                else if(filename == "files.xml")
                    sendFilelist(false, false);
            }
            //$ADCGET file TTH/YBV3TWWYW7ZWN3UWXFI4RJV6G54DTY62GE4VPNA 0 65536|
            else if(list.at(1) == "file")
            {
                QString tth = list.at(2);
                if(tth.contains("TTH/")) // TTH
                {
                    tth.remove(0,4);
                }
                else // filename
                {
                }
                quint64 start = list.at(3).toULongLong();
                quint64 len = list.at(4).toULongLong();
                if(len == 0) return;
                QString flags;
                if(list.size() == 6)
                    flags = list.at(5);
                sendFile(tth, start, len, flags);
            }
            else if(list.at(1) == "list")
            {
                sendFilelist(false, true);
            }
            // $ADCGET tthl TTH/5BBA6CQAVIKAV3UWXFI4RJV6G54DTY62GE4VPNA 0 -1
            else if(list.at(1) == "tthl")
            {
                QString tth = list.at(2);
                tth.remove(0,4);

                quint64 start = list.at(3).toULongLong();
                quint64 len = list.at(4).toULongLong();

                QString flags;
                if(list.size() == 6)
                    flags = list.at(5);

                sendTthl(tth, start, len, flags);
            }
        }
    }
}
void ClientConnection::sendFile(QString tth, quint64 start, quint64 len, QString flags)
{
    if(uploadTTH != tth)
    {        
        QSqlDatabase db = QSqlDatabase::database("upload"+username, true);
        if(!db.isValid())
        {
            db = QSqlDatabase::addDatabase("QSQLITE", "upload"+username);

            db.setDatabaseName("files.sqlite");
            if(!db.open())
                qDebug() << "Error opening database in SendTTHL";
        }

        QSqlQuery query(db);
        query.prepare("SELECT files.filename, directories.pathAbs FROM files INNER JOIN directories ON files.directory_id = directories.id WHERE files.TTH=:tth");
        query.bindValue(":tth", tth);

        if(!query.exec()) {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
            return;
        }
        // checking is file in db?
        if(query.next())
        {
            if(uploadFileTTH.isOpen()) uploadFileTTH.close();
            uploadFileTTH.setFileName(QDir(query.value(1).toString()).absoluteFilePath(query.value(0).toString()));
        }
        else
            return;
        uploadTTH = tth;
    }
    if(!uploadFileTTH.isOpen()) {
        if(!uploadFileTTH.open(QIODevice::ReadOnly))
        {
            qDebug() << "error opening upload file" << uploadFileTTH.fileName();
            return;
        }
    }

    if((start + len) > uploadFileTTH.size()) return;

    QByteArray response;

    response.append(QString("$ADCSND file TTH/%1 %2 %3|").arg(tth).arg(start).arg(len));
    emit signal_tcp_write(response);

    uploadFileTTH.seek(start);
    response.clear();
    response = uploadFileTTH.read(len);

    emit signal_tcp_write(response);

}
void ClientConnection::sendTthl(QString tth, quint64 start, quint64 len, QString flags)
{
    // Searching directories
    QByteArray tthl;
    QSqlDatabase db = QSqlDatabase::database("upload"+username, true);
    if(!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "upload"+username);

        db.setDatabaseName("files.sqlite");
        if(!db.open())
            qDebug() << "Error opening database in SendTTHL";
    }

    QSqlQuery query(db);
    query.prepare("SELECT interleaves FROM files WHERE TTH = :tth");
    query.bindValue(":tth", tth);

    if(!query.exec()) {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
        return;
    }
    // checking is tthl in db?
    if(query.next())
    {
        tthl = query.value(0).toByteArray();
    }
    else
    {
        return;
    }

    QByteArray response;
    if(len == 0) len = tthl.size() - start;

    response.append(QString("$ADCSND tthl TTH/%1 %2 %3|").arg(tth).arg(start).arg(len));
    emit signal_tcp_write(response);

    QByteArray res = tthl.mid(start, len);

    emit signal_tcp_write(tthl);
}
void ClientConnection::sendFilelist(bool isBZ2, bool isList)
{
    QString filename;
    if(isBZ2)
        filename = "files.xml.bz2";
    else
        filename = "files.xml";

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) return;

    QByteArray response;

    if(!isList) response.append(QString("$ADCSND file files.xml.bz2 0 %1|").arg(file.size()));
    else response.append(QString("$ADCSND list / 0 %1|").arg(file.size()));

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
