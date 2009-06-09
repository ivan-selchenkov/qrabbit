#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QtCore>
#include "fileinfo.h"
#include "dirstree.h"
#include "initfiletree.h"
#include "searchitem.h"

class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager(QObject* parent);
    ~FileManager();
    void initScan();
private:
    quint64 totalCount;
    InitFileTree* ift;
    QList<DirsTree> tree;
    QList<QDir> folders;


    //! Searching files & directories
    void searchStart(SearchItem& search);
    //! Searching in subdirectories
    void searchDirectory(SearchItem&, DirsTree &realTree);

public slots:
    void onHasingStatus(quint64);
    void on_search_request(QString search, QString mark);
    void on_search_finished(QList<FileInfo> list, QString mark);
private slots:
    void on_init_scan();
signals:
    //! Signal for MainWindow - new info for status bar
    void newInfo(QString);
    //! Signal for MainWindow - hashing progress
    void progressInfo(int);
    void searchFinished(QList<FileInfo> itemslist, QString mark);
};
#endif // FILEMANAGER_H
