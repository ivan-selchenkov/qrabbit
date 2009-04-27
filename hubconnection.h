#ifndef HUBCONNECTION_H
#define HUBCONNECTION_H

#include <QTcpSocket>
#include <QUdpSocket>
#include <QMutex>
#include <QList>
#include "hubnicklist.h"
#include "tablemodel.h"

class HubNickList;
class TableModel;
struct UdpDatagram;

class HubConnection: public QObject
{
    Q_OBJECT
public:
    TableModel* model;
    HubConnection(QObject* parent, QString, quint16);
    QString userName;
    QString password;
    QString Host;
    quint16 Port;
    quint16 slotsNumber;
    QString email;
    char* encoding;
    QString hubTopic;
    QString hubName;
    HubNickList* nicklist;
    bool isConnected();
    QByteArray changeKeysStC(QByteArray);
    QByteArray changeKeysCtS(QByteArray);
    QString decode(QByteArray);
    QByteArray encode(QString);
    bool isExtended();
    void SendMessage(QString);
    void SendSearch(QString);
public slots:
    void slotConnect();
    void slotReadyRead();
    void slotConnected();
    void slotParseList();
    void slotStartSend();
    void slotLoadNickList();
    void slotStartSendUdp();
    void slotSearchMessage(QString);
private:
    QTcpSocket* m_pTcpSocket;
    QUdpSocket* m_pUdpSocket;
    QByteArray buffer;
    QMutex m_mutex;
    QList<QByteArray> fromServer;
    QList<QByteArray> toServer;
    QList<UdpDatagram> outUdp;
    QList<UdpDatagram> inUdp;
    bool isListParsing;
    bool isSending;
    bool isSendingUdp;

    bool isHello;
    QTextCodec *codec;

    void splitArray();
    QByteArray generateKey(const QByteArray&);
    bool m_isExtended;
signals:
    void signalConnected();
    void signalDisconnected();
    void signalRedirect(QString strHost, quint16 nPort);
    void signalParseList();
    void signalStartSend();
    void signalDisplayMessage(QString&);

    // Сигналы для списка пользователей
    void signalNickList(QByteArray);
    void signalHello(QByteArray);
    void signalQuit(QString);
    void signalMyINFO(QString data);
    void signalListChanged();
    void signalListAboutChanged();

    void signalSearchMessage(QString);
    void signalStartSendUdp();
};
struct UdpDatagram
{
    QByteArray data;
    QString host;
    int port;
};
enum Ftype
{
    ANY = 1,
    AUDIO = 2,
    ARCHIVE = 3,
    DOCUMENT = 4,
    BIN = 5,
    IMAGE = 6,
    VIDEO = 7,
    FOLDER = 8,
    TTH = 9,
    ISO = 10
};
struct SearchPack
{
    QString host;
    QString nick;
    int port;
    bool isLimit;
    bool isMore;
    long size;
    Ftype type;
    QString data;
    QString toString()
    {
        return QString("Host: %3, Port: %4, Nick: %5, Search: %1, Type: %2").arg(data).arg(type).arg(host).arg(port).arg(nick);
    }
};
#endif // HUBCONNECTION_H
