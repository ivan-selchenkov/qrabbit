#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QSettings>
#include "hubthreadcontrol.h"
//#include "filemanager.h"
#include "initfilestree.h"
#include "fileinfo.h"
#include "searchthreadcontrol.h"

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
//    FileManager* filemanager;
    InitFilesTree* ift;
    SearchThreadControl* stc;
    QList<HubThreadControl*> hubs;
    Ui::MainWindowClass *ui;
    QSettings settings;
    QList<QString> folders;
public slots:
    void slotDisplayMessages(QString);
    void on_info(QString);
    void slot_on_progress_info(int);
    void on_hashing_finished();
    void on_search_result(FileInfo, QString);
    void slot_new_sharesize(quint64);
private slots:
    void on_lineSearch_returnPressed();
    void on_lineEdit_returnPressed();
    void on_btnStart_clicked();

    void slot_hub_destroyed(QObject*);
};

#endif // MAINWINDOW_H
