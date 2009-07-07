#include "hubconnection.h"
#include "global.h"
#include <QTextCodec>
#include <QStringList>
#include "searchitem.h"

HubConnection::HubConnection(QObject* parent, QString str_Host, quint16 n_Port): QObject(parent)
{
    //connect(this, SIGNAL(signalParseList()), SLOT(slotParseList()));
    //connect(this, SIGNAL(signalStartSend()), SLOT(slotStartSend()));

    isListParsing = false; // Метка что анализатор не запущен
    isSending = false; // Метка что отправка не запущена
    isSendingUdp = false;
    isHello = false;
    userName = "Washik";
    password = "vanqn1982";
    Host = str_Host;
    Port = n_Port;
    slotsNumber = 10;
    email = "rabbit@ya.ru";
    encoding = (char*)"windows-1251";
    nicklist = new HubNickList(userName, this);
    model = new TableModel(&(nicklist->list), this);

    connect(nicklist, SIGNAL(signalListAboutToBeChanged()), this, SIGNAL(signalListAboutChanged()));
    connect(nicklist, SIGNAL(signalListChanged()), this, SIGNAL(signalListChanged()));

    connect(this, SIGNAL(signalListAboutChanged()), model, SIGNAL(layoutAboutToBeChanged()));
    connect(this, SIGNAL(signalListChanged()), model, SIGNAL(layoutChanged()));

    connect(this, SIGNAL(signalMyINFO(QString)), nicklist, SLOT(slotMyInfo(QString)));
    connect(this, SIGNAL(signalQuit(QString)), nicklist, SLOT(slotQuitMessage(QString)));

    //connect(this, SIGNAL(signalSearchMessage(QString)), this, SLOT(slot_search_message(QString)));

    connect(this, SIGNAL(signalStartSendUdp()), this, SLOT(slotStartSendUdp()));
    codec = QTextCodec::codecForName(encoding); // кодек для перекодировки сообщений
    m_isExtended = false;

    hubtcpsocket = new HubTcpSocket(this);
    connect(hubtcpsocket, SIGNAL(signal_command_received(QByteArray)), this, SLOT(slot_command_received(QByteArray)));
    connect(this, SIGNAL(signal_tcp_write(QByteArray)), hubtcpsocket, SLOT(slot_write(QByteArray)));

    hubudpsocket = new HubUdpSocket(this);
    connect(this, SIGNAL(signal_udp_write(QByteArray,QString,quint16)), hubudpsocket, SLOT(slot_write(QByteArray,QString,quint16)));
}
void HubConnection::slotConnect()
{
    hubtcpsocket->connectToHost(Host, Port);
}
void HubConnection::slotConnected()
{
    qDebug() << "[INFO] Received the connected() signal";
}
bool HubConnection::isExtended()
{
    return m_isExtended;
}

void HubConnection::slot_command_received(QByteArray current)
{
    QList<QByteArray> list;
    QByteArray result;
    QString tempstr;
    QByteArray lock;

    if(current.isEmpty()) return; // Если пустая строка идем к следующей

    qDebug()<<"[DATA] "+current;
    if(current.at(0) == '$') {
        list = current.split(' '); // Делим строку на части по пробелу
        // Анализируем первую часть
        if(list.at(0) == "$Lock") { // Сервер послал $Lock
            result.clear();
            if(current.contains("EXTENDEDPROTOCOL"))
            {
                m_isExtended = true;
                result.append("$Supports UserCommand NoGetINFO NoHello UserIP2 TTHSearch ZPipe0|");
                lock = list.at(1);
                lock = lock.remove(0, 16);
            }
            else
                lock = list.at(1);
            qDebug() << "Key = "<< lock;
            result.append("$Key ");
            result.append(generateKey(list.at(1)));
            result.append('|');
            emit signal_tcp_write(result);

            result.clear();
            result.append("$ValidateNick ");
            result.append(userName);
            result.append('|');

            emit signal_tcp_write(result);
        }
        // Представляем расширенную инфорамция для регистрации на хабе
        else if(list.at(0) == "$Hello" && list.at(1) == userName && isHello == false) {
            result.clear();
            result.append("$Version 1.0091|");
            emit signal_tcp_write(result);

            result.clear();
            result.append("$GetNickList|");
            emit signal_tcp_write(result);

            result.clear();
            result.append("$MyINFO $ALL ");
            result.append(userName);
            result.append(QString(" <Rabbit++ V:%1,M:A,H:1/0/0,S:%2>$ $1000%4$%3$%5$|").arg(VERSION).arg(slotsNumber).arg(email).arg((char)0x1).arg(sharesize));
            emit signal_tcp_write(result);
            isHello = true;
        }        
        else if(list.at(0) == "$GetPass")
        {
            result.clear();
            result.append("$MyPass ");
            result.append(password);
            result.append("|");
            emit signal_tcp_write(result);
        }
        // Пользователь ушел
        else if(list.at(0) == "$Quit")
        {
            tempstr = decode(current.mid(6, current.size() - 6));
            emit signalQuit(tempstr);
        }
        // Принимаем топик хаба
        else if(list.at(0) == "$HubTopic")
        {
            current = current.mid(10, current.size() - 10);
            hubTopic = decode(changeKeysStC(current));
        }
        // Принимаем имя хаба
        else if(list.at(0) == "$HubName")
        {
            current = current.mid(9, current.size() - 9);
            hubName = decode(changeKeysStC(current));
        }
        // Сигнал что хаб полный
        else if(list.at(0) == "$HubIsFull")
        {
            hubtcpsocket->close();
            emit signalDisconnected();
        }
        // Переход на другой хаб
        else if(list.at(0) == "$ForceMove")
        {
            if(list.at(1) == "") {
                hubtcpsocket->close();
                emit signalDisconnected();
            }
            else
            {
                tempstr = "Redirect to " + list.at(1);                
                emit signalDisplayMessage(tempstr);
                QStringList t_str = tempstr.split(":");
                if(t_str.size() == 2)
                {
                    emit signalRedirect(t_str.at(0), t_str.at(1).toInt());
                }
            }
        }
        else if(list.at(0) == "$MyINFO")
        {
            tempstr = decode(current);
            tempstr.remove("$MyINFO $ALL ");
            emit signalMyINFO(tempstr);
        }
        else if(list.at(0) == "$Search")
        {
            tempstr = decode(changeKeysStC(current));
            tempstr = tempstr.remove("$Search ");
            searchMessage(tempstr);
        }
        else if(list.at(0) == "$ConnectToMe")
        {
            QString address;
            QString username;

            if(list.size() == 4) // new protocol
            {
                //username = decode(changeKeysStC(list.at(2)));
                username = list.at(2);
                address = list.at(3);
                emit slot_new_client(username, address);
            }
            else // old protocol
            {
                //username = decode(changeKeysStC(list.at(1)));
                username = list.at(1);
                address = list.at(2);
                emit slot_new_client(username, address);
            }
        }
    } else {
        tempstr = decode(changeKeysStC(current));
        emit signalDisplayMessage(tempstr);
    }
}
void HubConnection::slot_set_sharesize(quint64 size)
{
    QByteArray result;
    sharesize = size;

    if(hubtcpsocket->isOpen())
    {
        result.clear();
        result.append("$MyINFO $ALL ");
        result.append(userName);
        result.append(QString(" <Rabbit++ V:%1,M:A,H:1/0/0,S:%2>$ $1000%4$%3$%5$|").arg(VERSION).arg(slotsNumber).arg(email).arg((char)0x1).arg(sharesize));
        emit signal_tcp_write(result);
    }
}
void HubConnection::slot_new_client(QString username, QString address)
{
    QByteArray data;
    data.append(QString("$ConnectToMe %1 %2|").arg(username).arg(address));
    ClientConnection* client = new ClientConnection(this, username, address, data);
    client_list.append(client);
}
void HubConnection::searchMessage(QString search)
{
    //qDebug() << search;

    SearchItem search_item;

    QStringList split = search.split(" ");

    if(split.size() < 2) return;
    QStringList split_ip = split.at(0).split(":");

    if(split_ip.size() < 2) return;

    if(split_ip.at(0) == "Hub") // user is passive, send answer to hub
    {
        search_item.host = "Hub";
        search_item.nick = split_ip.at(1);
    }
    else
    {
        search_item.host = split_ip.at(0);
        search_item.port = split_ip.at(1).toInt();
    }

    QStringList split_s = split.at(1).split("?");
    if(split_s.size() < 5) return;

    if(split_s.at(0) == "T") search_item.isLimit = true;
    else search_item.isLimit = false;

    if(split_s.at(1) == "F") search_item.isMore = true;
    else search_item.isMore = false;

    search_item.size = split_s.at(2).toLong();
    search_item.type = (SearchItem::SEARCH_FILE_TYPE)split_s.at(3).toInt();

    search_item.data = split_s.at(4);

    qRegisterMetaType<SearchItem>("SearchItem");

    emit signal_search_request(search_item);

}
void HubConnection::slot_search_result(FileInfo file_info, SearchItem search_item)
{
    UdpDatagram udp;
    QString answer;
// $SR fromuser path_to_file.ext0x05filesize freeslots/openslots0x05hubname (hubaddress[:port])0x05touser|
// $SR fromuser path_to_file.ext0x05filesize freeslots/openslots0x05hubname (hubaddress[:port])|
// Directories: $SR <nick><0x20><directory><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
// Files: $SR <nick><0x20><filename><0x05><filesize><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
// $SR <user name> <file name>\05<file size> <slots>\05TTH:<file tth> (<hub ip>:<hub port>)|
//note that port only needs to specified if it's not 411
//There is also a SR command for answering active-mode queries via UDP.


    if(file_info.isDir) {
        answer = QString("$SR %1 %2").arg(userName).arg(file_info.relativePath.replace("/", "\\"));
    }
    else
    {
        answer = QString("$SR %1 %2").arg(userName).arg(file_info.relativePath.replace("/", "\\"));
        answer.append((char)0x05); // file
        answer.append(QString::number(file_info.size)); //file
    }


    answer.append(" 1/1"); //qDebug()<<"set correct slot number HubConnection::slot_search_result()";

    answer.append((char)0x05);

    if(!file_info.isDir) answer.append(QString("TTH:%1").arg(file_info.TTH));

    answer.append(QString(" (%2").arg(Host));

    if(Port != 411)
        answer.append(QString(":%1").arg(Port));

    answer.append(")");

    if(search_item.host == "Hub")
    {
        answer.append((char)0x05);
        answer.append(search_item.nick);
    }
    answer.append("|");
    udp.data.append(answer);
    udp.host = search_item.host;
    udp.port = search_item.port;

    if(search_item.host == "Hub")
        emit signal_tcp_write(udp.data);
    else
        emit signal_udp_write(udp.data, udp.host, udp.port);
}
void HubConnection::SendMessage(QString message)
{
    QByteArray send_array;
    message = QString("<%1> %2").arg(userName, message);
    send_array = changeKeysCtS(encode(message));
    send_array.append('|');
    emit signal_tcp_write(send_array);
}
QByteArray HubConnection::generateKey(const QByteArray& lock)
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
void HubConnection::SendSearch(QString search)
{
    QByteArray result;
    QString str = "$Search 127.0.0.1:55555 F?F?0?1?readme.txt|";
    result.clear();
    result.append(str);
    emit signal_tcp_write(result);
}
bool HubConnection::isConnected()
{
    return hubtcpsocket->isOpen();
}
QByteArray HubConnection::changeKeysStC(QByteArray ar) // from server to client
{
    QVector<QString> str;
    QVector<char> c;
    str << "&#0;" <<"&#5;"<<"&#36;"<<"&#96;"<<"&#124;"<<"&#126;"<<"&amp;";
    c << 0 << 5 << 36 << 96 << 124 << 126 << '&';

    for(int i = 0; i < str.size(); i++)
        ar.replace(str[i], &c[i]);
    return ar;
}

QByteArray HubConnection::changeKeysCtS(QByteArray ar) // from server to client
{
    QVector<QString> str;
    QVector<char> c;
    str << "&#0;" <<"&#5;"<<"&#36;"<<"&#96;"<<"&#124;"<<"&#126;"<<"&amp;";
    c << 0 << 5 << 36 << 96 << 124 << 126 << '&';

    for(int i = 0; i < str.size(); i++)
        ar.replace(c[i], str[i]);
    return ar;
}
QByteArray HubConnection::encode(QString str)  // coding text from client to server encoding
{
    return codec->fromUnicode(str);
}
QString HubConnection::decode(QByteArray ar)  // coding text from server to utf-8
{
    return codec->toUnicode(ar);
}
