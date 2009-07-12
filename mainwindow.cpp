#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchitem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);

    //    filemanager = new FileManager(this);
//    connect(filemanager, SIGNAL(signal_search_result(FileInfo,QString)), this, SLOT(on_search_result(FileInfo,QString)));
//    connect(filemanager, SIGNAL(signal_new_sharesize(quint64)), this, SLOT(slot_new_sharesize(quint64)));
    folders.append("/home/ivan/Downloads");

    ift = new InitFilesTree(this, folders);

    // Setting flag that file tree is ready for searching
//    connect(ift, SIGNAL(signal_finished()), this, SLOT(slot_on_hashing_finished()));
    // Sending hashing progress in percents
    connect(ift, SIGNAL(signal_hashing_progress(int)), this, SLOT(slot_on_progress_info(int)));
    connect(ift, SIGNAL(signal_new_sharesize(quint64)), this, SLOT(slot_new_sharesize(quint64)));

    ift->start(QThread::LowPriority);

    stc = new SearchThreadControl();
    stc->start(QThread::LowestPriority);
}

MainWindow::~MainWindow()
{    
    ift->terminate(); // terminating hashing process
    ift->wait(); // waiting while terminating...
    delete ift;

    stc->exit();
    stc->wait();

    foreach(HubThreadControl* hub, hubs)
    {
        hub->exit();
        hub->wait();
        delete hub;
    }
    delete stc;
    delete ui;
}

void MainWindow::on_search_result(FileInfo fi, QString mark)
{
    slotDisplayMessages(fi.dir.absoluteFilePath(fi.filename));
}
void MainWindow::on_btnStart_clicked()
{
//    connect(s, SIGNAL(progressInfo(int)), this, SLOT(on_progress_info(int)));

    HubThreadControl* hub;

    //hub = new HubThreadControl("dc.xoxloma.net", 411, "Washik", "vanqn1982", "10.5.2.51", 10, "rabbit@ya.ru", "cp1251"); // dc.wideix.ru warez.gtk.su
    //hub = new HubThreadControl("dc.wideix.ru", 411, "Washik", "vanqn1982", "10.5.2.51", 10, "rabbit@ya.ru", "cp1251"); // dc.wideix.ru warez.gtk.su
    hub = new HubThreadControl("localhost", 411, "Washik", "vanqn1982", "192.168.1.2", 10, "rabbit@ya.ru", "cp1251"); // dc.wideix.ru warez.gtk.su

    if(settings.contains("Sharesize"))
        hub->setSharesize(settings.value("sharesize").toULongLong());

    connect(hub, SIGNAL(signal_hub_message(QString)),
            this, SLOT(slotDisplayMessages(QString)));

    connect(hub, SIGNAL(signal_search_request(SearchItem)),
            stc, SIGNAL(signal_income_search(SearchItem)));

    // connecting hubconnection to filemanager to analize search request
    connect(stc, SIGNAL(signal_outcome_search(FileInfo,SearchItem)),
            hub, SIGNAL(signal_search_result(FileInfo,SearchItem)));


    connect(ift, SIGNAL(signal_new_sharesize(quint64)),
            hub, SIGNAL(signal_sharesize(quint64)));

    connect(hub,SIGNAL(destroyed(QObject*)), this, SLOT(slot_hub_destroyed(QObject*)));

    hub->start();


    ui->tableView->setModel(hub->model);
    ui->tableView->setColumnWidth (0, 150 ); //Nick size
    ui->tableView->setColumnWidth (1, 100 ); // Share size
    ui->tableView->setColumnWidth (2, 200 ); // Desc size

    hubs.append(hub);
}
void MainWindow::slot_hub_destroyed(QObject* obj)
{
    HubThreadControl* hub = (HubThreadControl*) obj;
    qDebug() << "Hub destroyed...";
    hubs.removeOne(hub);
}
void MainWindow::slot_new_sharesize(quint64 size)
{
    settings.setValue("sharesize", size);
}
void MainWindow::slotDisplayMessages(QString str)
{
    ui->plainTextEdit->appendPlainText(str);
}

void MainWindow::on_lineEdit_returnPressed()
{
    QString str = ui->lineEdit->text();
    if(str != "") {
        ui->lineEdit->clear();
            hubs[0]->sendMessage(str);
    }
}

void MainWindow::on_lineSearch_returnPressed()
{
    //filemanager->search(ui->lineSearch->text(), ui->lineSearch->text() + QTime::currentTime().toString());
   /* if(hubs[0]->isConnected())
    {
        hubs[0]->SendSearch("");
    } */

}
void MainWindow::on_info(QString str)
{    
    ui->lblInfo->setText(str);
    qApp->processEvents();
}
void MainWindow::slot_on_progress_info(int value)
{
    ui->lblInfo->setText(tr("Hashing: ") + QString::number(value) + " %");
}

void MainWindow::on_hashing_finished()
{
    ui->lblInfo->setText(tr("Hashing finished...OK"));
}
