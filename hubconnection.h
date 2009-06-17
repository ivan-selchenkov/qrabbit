#ifndef HUBCONNECTION_H
#define HUBCONNECTION_H

#include <QTcpSocket>
#include <QUdpSocket>
#include <QMutex>
#include <QList>
#include "hubnicklist.h"
#include "tablemodel.h"
#include "searchitem.h"
#include "fileinfo.h"

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
    void slotLoadNickList();
    void slotStartSendUdp();

    void slot_search_result(FileInfo, SearchItem);
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
    void searchMessage(QString);
    void parseList();
    void startSend();
signals:
    void signalConnected();
    void signalDisconnected();
    void signalRedirect(QString strHost, quint16 nPort);
    void signalDisplayMessage(QString&);

    // Сигналы для списка пользователей
    void signalNickList(QByteArray);
    void signalHello(QByteArray);
    void signalQuit(QString);
    void signalMyINFO(QString data);
    void signalListChanged();
    void signalListAboutChanged();

    //void signalSearchMessage(QString);
    void signalStartSendUdp();

    void signal_search_request(SearchItem);
};
struct UdpDatagram
{
    QByteArray data;
    QString host;
    int port;
};
#endif // HUBCONNECTION_H
