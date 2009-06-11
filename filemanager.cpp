#include "filemanager.h"
#include "mainwindow.h"

FileManager::FileManager(QObject* parent): QObject(parent), m_isFileListLoaded(false)
{
    folders.append(QDir("/home/ivan/Downloads"));

    ift = new InitFileTree(this, tree, folders);

    // Setting flag that file tree is ready for searching
    connect(ift, SIGNAL(signal_finished()), this, SLOT(slot_on_hashing_finished()));
    // Sending hashing progress in percents
    connect(ift, SIGNAL(signal_hashing_progress(int)), (MainWindow*) parent, SLOT(slot_on_progress_info(int)));

    ift->start(QThread::LowPriority);
}
FileManager::~FileManager()
{
    ift->terminate(); // terminating hashing process
    ift->wait(); // waiting while terminating...

    foreach(SearchManager* sm, searchHash)
    {
        sm->terminate();
        sm->wait();
    }
}
//! Hash is finished and file tree ready to search
void FileManager::slot_on_hashing_finished()
{
    m_isFileListLoaded = true;
}

void FileManager::slot_on_search_finished(QList<FileInfo> itemslist, QString mark)
{
    //qDebug() << itemslist[0].filename;
}

void FileManager::slot_on_search_request(QString search, QString mark)
{
    SearchItem si(search, mark);

    SearchManager sm(this, tree, si);
    searchHash["mark"] = &sm;
    sm.start(QThread::LowestPriority);
}





