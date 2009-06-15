#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QtCore>
#include "fileinfo.h"
#include "dirstree.h"
#include "initfiletree.h"
#include "searchitem.h"
#include "searchmanager.h"

class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager(QObject* parent);
    ~FileManager();
    bool isFileListLoaded()
    {
        return m_isFileListLoaded;
    }
    void search(QString search, QString mark);
private:
    InitFileTree* ift;
    QList<DirsTree> tree;
    QList<QDir> folders;
    QHash<QString, SearchManager*> searchHash;

    bool m_isFileListLoaded;
public slots:
    void slot_on_search_finished(QString mark);
    void slot_on_search_result(FileInfo, QString mark);
private slots:
    void slot_on_hashing_finished();
signals:
    //! Signal for MainWindow - new info for status bar
    void newInfo(QString);
    //! Signal for MainWindow - hashing progress
    void progressInfo(int);
    void signal_search_result(FileInfo list, QString mark);
};
#endif // FILEMANAGER_H
