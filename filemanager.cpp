#include "filemanager.h"
#include "mainwindow.h"
#include <QThreadPool>

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
    qDebug() << "~FileManager()";

    ift->terminate(); // terminating hashing process
    ift->wait(); // waiting while terminating...
    delete ift;

    /*
    foreach(SearchManager* sm, searchHash)
    {
        sm->terminate();
        sm->wait();
    }
    */
}
//! Hash is finished and file tree ready to search
void FileManager::slot_on_hashing_finished()
{
    m_isFileListLoaded = true;
}
void FileManager::slot_on_search_result(FileInfo fi, QString mark)
{
    emit signal_search_result(fi, mark);
}
void FileManager::slot_on_search_finished(QString mark)
{
    //SearchManager* sm = searchHash[mark];
    //delete sm;
    //searchHash.remove(mark);
}

void FileManager::search(QString search, QString mark)
{
    SearchManager* sm = new SearchManager(this, tree, search, mark);

    qRegisterMetaType<FileInfo>("FileInfo");

    connect(sm, SIGNAL(signal_search_result(FileInfo,QString)), this, SLOT(slot_on_search_result(FileInfo,QString)));
    //connect(sm, SIGNAL(signal_search_finished(QString)), this, SLOT(slot_on_search_finished(QString)));

    //searchHash[mark] = sm;
    //sm->start(QThread::LowestPriority);
    QThreadPool::globalInstance()->start(sm);
}





