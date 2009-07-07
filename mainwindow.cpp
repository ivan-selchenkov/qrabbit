#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchitem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);
    filemanager = new FileManager(this);
    //connect(filemanager, SIGNAL(signal_search_result(FileInfo,QString)), this, SLOT(on_search_result(FileInfo,QString)));
    connect(filemanager, SIGNAL(signal_new_sharesize(quint64)), this, SLOT(slot_new_sharesize(quint64)));

    stc = new SearchThreadControl();
    stc->start(QThread::LowestPriority);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_search_result(FileInfo fi, QString mark)
{
    slotDisplayMessages(fi.dir.absoluteFilePath(fi.filename));
}
void MainWindow::on_btnStart_clicked()
{
//    connect(s, SIGNAL(progressInfo(int)), this, SLOT(on_progress_info(int)));

    HubConnection* hub;

//    hub = new HubConnection(this, "dc.wideix.ru", 411); // dc.wideix.ru warez.gtk.su
    hub = new HubConnection(this, "192.168.1.2", 411); // dc.wideix.ru warez.gtk.su
    if(settings.contains("Sharesize"))
    {
        hub->slot_set_sharesize(settings.value("sharesize").toULongLong());
    }
    hub->slotConnect();
    connect(hub, SIGNAL(signalDisplayMessage(QString&)),
            this, SLOT(slotDisplayMessages(QString&)));

    // connecting hubconnection to filemanager to analize search request
    connect(stc, SIGNAL(signal_outcome_search(FileInfo,SearchItem)),
            hub, SLOT(slot_search_result(FileInfo,SearchItem)), Qt::AutoConnection);

    connect(hub, SIGNAL(signal_search_request(SearchItem)),
            stc, SIGNAL(signal_income_search(SearchItem)), Qt::AutoConnection);

    connect(filemanager, SIGNAL(signal_new_sharesize(quint64)),
            hub, SLOT(slot_new_sharesize(quint64)));


    ui->tableView->setModel(hub->model);
    ui->tableView->setColumnWidth (0, 150 ); //Nick size
    ui->tableView->setColumnWidth (1, 100 ); // Share size
    ui->tableView->setColumnWidth (2, 200 ); // Desc size

    hubs.append(hub);
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
        if(hubs[0]->isConnected())
        {
            hubs[0]->SendMessage(str);
        }
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
