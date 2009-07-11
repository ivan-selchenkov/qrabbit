#include "hubconnection.h"
#include "global.h"
#include <QTextCodec>
#include <QStringList>
#include "searchitem.h"
#include <iconv.h>
#include <string.h>
#include <limits.h>

HubConnection::HubConnection(QString str_Host, quint16 n_Port)
{
    isListParsing = false; // Метка что анализатор не запущен
    isSending = false; // Метка что отправка не запущена
    isSendingUdp = false;
    isHello = false;
    m_isExtended = false;

    Host = str_Host;
    Port = n_Port;
    qDebug() << "   HubConnection()";
}
bool HubConnection::init()
{
    if(userName.isEmpty()) return false;
    //password = "vanqn1982";
    if(localHost.isEmpty()) return false;;
    if(slotsNumber <= 0 && slotsNumber > 100) slotsNumber = 10;
    if(email.isEmpty()) email = "rabbit@ya.ru";
    if(encoding.isEmpty()) return false; // (char*)"windows-1251";


    // codec for encoding messages
    //codecParent = new QTextCodec();
    //codec = codecParent->codecForName((char*)encoding.constData());

    // hub tcp read/write control
    hubtcpsocket = new HubTcpSocket(this);
    // incoming command
    connect(hubtcpsocket, SIGNAL(signal_command_received(QByteArray)),
            this, SLOT(slot_command_received(QByteArray)));
    // outcoming data
    connect(this, SIGNAL(signal_tcp_write(QByteArray)),
            hubtcpsocket, SLOT(slot_write(QByteArray)));

    // hub/client udp write control
    hubudpsocket = new HubUdpSocket(this);
    connect(this, SIGNAL(signal_udp_write(QByteArray,QString,quint16)),
            hubudpsocket, SLOT(slot_write(QByteArray,QString,quint16)));
    qDebug() << "   HubConnection::init()";
    return true;
}
HubConnection::~HubConnection()
{
    qDebug() << "~HubConnection()";
}

void HubConnection::connectToHub()
{
    qDebug() << Host << ":" << Port;
    hubtcpsocket->connectToHost(Host, Port);
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

    if(current.isEmpty()) return;

    qDebug()<<"[DATA] "+decode(changeKeysStC(current));
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
                emit signal_hub_message(tempstr);
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
            qDebug()<<"[DATA] "+current;
            QString address;
            QString username;

            if(list.size() == 4) // new protocol
            {
                //username = decode(changeKeysStC(list.at(2)));
                username = list.at(2);
                address = list.at(3);
                newClient(username, address, true);
            }
            else // old protocol
            {
                //username = decode(changeKeysStC(list.at(1)));
                username = list.at(1);
                address = list.at(2);
                newClient(username, address, true);
            }
        }
        else if(list.at(0) == "$RevConnectToMe")
        {
            if(list.size() < 1)
                return;
            QString username = list.at(1);
            newClient(username, localHost, false);
            qDebug()<<"[DATA] "+current;

        }
    } else {
        tempstr = decode(changeKeysStC(current));
        emit signal_hub_message(tempstr);
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
void HubConnection::newClient(QString username, QString address, bool isActive)
{
    QByteArray data;
    if(isActive)
        data.append(QString("$ConnectToMe %1 %2|").arg(username).arg(address));
    else
    {
        data.append(QString("$RevConnectToMe %1|").arg(username));
        for(int i=0; i<client_list.size(); i++)
        {
            if(client_list.at(i)->username == username) return;
        }
    }
    ClientConnection* client = new ClientConnection(this, username, address, data, isActive);
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

    if(search_item.data.size() >= 3)
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

    qDebug() << "Search data:" << search_item.data;

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
void HubConnection::slot_send_message(QString message)
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
    QByteArray result;
    QByteArray ar = str.toUtf8();

    iconv_t cd;
    size_t ret;

    char* data = ar.data();
    size_t insize = (size_t)ar.size();

    char* out = new char[(insize + 1) * MB_LEN_MAX];
    char* presult = out;
    size_t outsize = (insize + 1) * MB_LEN_MAX - 1;
    size_t outsizeleft = outsize;

    QByteArray ascii = encoding.toAscii();

    cd = iconv_open(ascii.constData(), "utf8");

    ret = iconv(cd, &data, &insize, &out, &outsizeleft);
    iconv_close(cd);
    if(ret == (size_t)(-1))
    {
        qDebug() << "iconv error";
        return QByteArray();
    }
    result = QByteArray::fromRawData(presult, (int) outsize - outsizeleft);
    return  result;
}
QString HubConnection::decode(QByteArray ar)  // coding text from server to utf-8
{
//    iconv_t iconv_open(const char *tocode, const char *fromcode);
    QString str;
    iconv_t cd;
    size_t ret;

    char* data = ar.data();
    size_t insize = (size_t)ar.size();

    char* out = new char[(insize + 1) * MB_LEN_MAX];
    char* result = out;
    size_t outsize = (insize + 1) * MB_LEN_MAX - 1;
    size_t outsizeleft = outsize;

    QByteArray ascii = encoding.toAscii();

    cd = iconv_open("utf8", ascii.constData());

    ret = iconv(cd, &data, &insize, &out, &outsizeleft);
    iconv_close(cd);
    if(ret == (size_t)(-1))
    {
        qDebug() << "iconv error";
        return QString();
    }
    str = QString::fromUtf8(result, (int) outsize - outsizeleft);
    return str;
}
