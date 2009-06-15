#include "hubconnection.h"
#include "global.h"
#include <QTextCodec>
#include <QStringList>

HubConnection::HubConnection(QObject* parent, QString str_Host, quint16 n_Port): QObject(parent)
{
    connect(this, SIGNAL(signalParseList()), SLOT(slotParseList()));
    connect(this, SIGNAL(signalStartSend()), SLOT(slotStartSend()));
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

    m_pUdpSocket = new QUdpSocket(this);
}
void HubConnection::slotConnect()
{
    m_pTcpSocket = new QTcpSocket(this);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    m_pTcpSocket->connectToHost(Host, Port);
}
void HubConnection::slotConnected()
{
    qDebug() << "[INFO] Received the connected() signal";
}
void HubConnection::slotReadyRead()
{
    QByteArray b;
    for(;;) {
        if(m_pTcpSocket->bytesAvailable() <= 0) // Если ничего нет, выходим
            break;
        b = m_pTcpSocket->readAll();// Читаем всё что есть
        m_mutex.lock();
            buffer.append(b);
        m_mutex.unlock();
        //qDebug() << b;
        splitArray();
    }
}
void HubConnection::splitArray()
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
    if(list_array.size() > 1) fromServer.append(list_array); // Добавляем в наш список
    parseList(); // Испускаем сигнал о том, что можно анализировать список

}
bool HubConnection::isExtended()
{
    return m_isExtended;
}
void HubConnection::parseList()
{
    if(isListParsing == true) return; // Если функция уже запущена

    isListParsing = true; // метка что функция запущена

    QByteArray current;
    QList<QByteArray> list;
    QByteArray result;
    QString tempstr;
    QByteArray lock;

    while(!fromServer.isEmpty()) { // Пока список не пуст
        current = fromServer.takeFirst(); // Берем первый и удаляем его
        if(current.isEmpty()) continue; // Если пустая строка идем к следующей

        //qDebug()<<"[DATA] "+current;
     if(current.at(0) == '$') {
        list = current.split(' '); // Делим строку на части по пробелу
        // Анализируем первую часть
        if(list.at(0) == "$Lock") { // Сервер послал $Lock
            result.clear();
            if(current.contains("EXTENDEDPROTOCOL"))
            {
                m_isExtended = true;
                result.append("$Supports UserCommand NoGetINFO NoHello UserIP2 TTHSearch ZPipe0 GetZBlock |$Supports OpPlus NoGetINFO NoHello UserIP2|");
                lock = list.at(1);
                lock = lock.remove(0, 16);
            }
            else
                lock = list.at(1);
            qDebug() << "Key = "<< lock;
            result.append("$Key ");
            result.append(generateKey(list.at(1)));
            result.append('|');
            toServer.append(result);

            result.clear();
            result.append("$ValidateNick ");
            result.append(userName);
            result.append('|');
            toServer.append(result);
            startSend();
        }
        // Представляем расширенную инфорамция для регистрации на хабе
        else if(list.at(0) == "$Hello" && list.at(1) == userName && isHello == false) {
            result.clear();
            result.append("$Version 0.01|");
            toServer.append(result);

            slotLoadNickList();
            result.clear();
            result.append("$MyINFO $ALL ");
            result.append(userName);
            result.append(QString(" <Rabbit++ V:%1,M:A,H:1/0/0,S:%2>$ $10Gb LAN(T3)$%3$10000000000$|").arg(VERSION).arg(slotsNumber).arg(email));
            toServer.append(result);
            isHello = true;
            startSend();
        }        
        else if(list.at(0) == "$GetPass")
        {
            result.clear();
            result.append("$MyPass ");
            result.append(password);
            result.append("|");
            toServer.append(result);
            startSend();
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
            m_pTcpSocket->close();
            emit signalDisconnected();
        }
        // Переход на другой хаб
        else if(list.at(0) == "$ForceMove")
        {
            if(list.at(1) == "") {
                m_pTcpSocket->close();
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
    } else {
        tempstr = decode(changeKeysStC(current));
        emit signalDisplayMessage(tempstr);
    }
  } // end while

    isListParsing = false; // снимает метку
}
void HubConnection::searchMessage(QString search)
{
    qDebug() << search;

    SearchPack s;
    QString answer;

    QStringList split = search.split(" ");

    if(split.size() < 2) return;
    QStringList split_ip = split.at(0).split(":");

    if(split_ip.size() < 2) return;

    if(split_ip.at(0) == "Hub") // user is passive, send answer to hub
    {
        s.host = "Hub";
        s.nick = split_ip.at(1);
    }
    else
    {
        s.host = split_ip.at(0);
        s.port = split_ip.at(1).toInt();
    }

    QStringList split_s = split.at(1).split("?");
    if(split_s.size() < 5) return;

    if(split_s.at(0) == "T") s.isLimit = true;
    else s.isLimit = false;

    if(split_s.at(1) == "F") s.isMore = true;
    else s.isMore = false;

    s.size = split_s.at(2).toLong();
    s.type = (Ftype)split_s.at(3).toInt();

    s.data = split_s.at(4);

//    UdpDatagram u;
//    u.data.clear();
    answer = QString("$SR Washik DC_SHARE\\%1.exe").arg(s.data);
    answer.append((char)0x05);
    answer.append("666");
    answer.append(" 1/1");
//    u.data.append(answer);
//    u.data.append((char)0x05);
    answer = QString("TTH:3CTFAAYAAAAAA4KW3RKKK7YAACQJM7YAAAAAAAA (127.0.0.1:411)|");
//    u.data.append(answer);
//    u.host = s.host;
//    u.port = s.port;

//    outUdp.append(u);

    emit signalStartSendUdp();

    QString tmp;
    tmp = s.toString();
    emit signalDisplayMessage(tmp);
}
void HubConnection::SendMessage(QString message)
{
    QByteArray send_array;
    message = QString("<%1> %2").arg(userName, message);
    send_array = changeKeysCtS(encode(message));
    send_array.append('|');
    toServer.append(send_array);
    startSend();
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
void HubConnection::startSend()
{
    QByteArray current;
    if(isSending == true) return; // Если функция уже запущена

    isSending = true; // метка что функция запущена
    while(!toServer.isEmpty()) { // Пока список не пуст
        current = toServer.takeFirst();
        m_pTcpSocket->write(current);
        qDebug()<<"[SEND] "+ current;
    }
    isSending = false; // снимает метку
}

void HubConnection::slotStartSendUdp()
{
    UdpDatagram current;
    int error;
    if(isSendingUdp == true) return; // Если функция уже запущена

    isSendingUdp = true; // метка что функция запущена
    while(!outUdp.isEmpty()) { // Пока список не пуст
        current = outUdp.takeFirst();
        error = m_pUdpSocket->writeDatagram(current.data, QHostAddress(current.host), current.port);
        qDebug()<<"[SEND UDP] "+ current.data;
    }
    isSendingUdp = false; // снимает метку
}
void HubConnection::SendSearch(QString search)
{
    QByteArray result;
    QString str = "$Search 127.0.0.1:55555 F?F?0?1?readme.txt|";
    result.clear();
    result.append(str);
    toServer.append(result);
    startSend();

}
bool HubConnection::isConnected()
{
    return m_pTcpSocket->isOpen();
}
// Requesting NickList
void HubConnection::slotLoadNickList()
{
    QByteArray result;
    result.append("$GetNickList|");
    toServer.append(result);
    startSend();
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
