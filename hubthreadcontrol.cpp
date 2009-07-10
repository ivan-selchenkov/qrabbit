#include "hubthreadcontrol.h"

HubThreadControl::HubThreadControl(QString m_hostname, quint16 m_port, QString m_username, QString m_password,
                                   QString m_localhost, int m_slots, QString m_email, QString m_encoding):
        hostname(m_hostname), port(m_port), username(m_username), password(m_password), localhost(m_localhost),
        slotsNumber(m_slots), email(m_email), encoding(m_encoding)
{

    // Hub's nicklist
    nicklistControl = new NicklistThreadControl(this, username);

    // Model for grid widget - nicklist
    model = new TableModel(nicklistControl, this);
     // signals for model
    connect(nicklistControl, SIGNAL(signal_list_about_to_be_changed()),
            model, SIGNAL(layoutAboutToBeChanged()));
    connect(nicklistControl, SIGNAL(signal_list_changed()),
            model, SIGNAL(layoutChanged()));
   // Starting hublist thread
    nicklistControl->start(QThread::LowestPriority);

    qDebug() << "HubThreadControl()";
}
HubThreadControl::~HubThreadControl()
{
    nicklistControl->exit();
    nicklistControl->wait();
    delete nicklistControl;
qDebug() << "~HubThreadControl()";
}
void HubThreadControl::setSharesize(quint64 size)
{
    emit signal_sharesize(size);
}
void HubThreadControl::run()
{
    hub = new HubConnection(hostname, port);

    // hub text messages
    connect(hub, SIGNAL(signal_hub_message(QString)),
            this, SIGNAL(signal_hub_message(QString)));
    // hub search requests
    connect(hub, SIGNAL(signal_search_request(SearchItem)),
            this, SIGNAL(signal_search_request(SearchItem)));
    // new sharesize
    connect(this, SIGNAL(signal_sharesize(quint64)),
            hub, SLOT(slot_set_sharesize(quint64)));
    // search result
    connect(this, SIGNAL(signal_search_result(FileInfo,SearchItem)),
            hub, SLOT(slot_search_result(FileInfo,SearchItem)));

    connect(hub, SIGNAL(signalMyINFO(QString)),
            nicklistControl, SIGNAL(signal_myinfo(QString)));
    connect(hub, SIGNAL(signalQuit(QString)),
            nicklistControl, SIGNAL(signal_quit(QString)));

    hub->userName = username;
    hub->password = password;
    hub->localHost = localhost;
    hub->slotsNumber = slotsNumber;
    hub->email = email;
    hub->encoding = encoding;
    hub->sharesize = 10737418240;
    hub->init();

    hub->connectToHub();

    exec();
}
