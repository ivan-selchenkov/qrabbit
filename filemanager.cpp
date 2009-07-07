#include "filemanager.h"
#include "mainwindow.h"
#include <QThreadPool>

FileManager::FileManager(QObject* parent): QObject(parent), m_isFileListLoaded(false)
{
    folders.append("/home/ivan/Downloads");
    //folders.append("/home/share/DC_SHARE");

    ift = new InitFilesTree(this, folders);

    // Setting flag that file tree is ready for searching
    connect(ift, SIGNAL(signal_finished()), this, SLOT(slot_on_hashing_finished()));
    // Sending hashing progress in percents
    connect(ift, SIGNAL(signal_hashing_progress(int)), (MainWindow*) parent, SLOT(slot_on_progress_info(int)));
    connect(ift, SIGNAL(signal_new_sharesize(quint64)), this, SIGNAL(signal_new_sharesize(quint64)));

    ift->start(QThread::LowPriority);
}
FileManager::~FileManager()
{
    qDebug() << "~FileManager()";

    ift->exit(); // terminating hashing process
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

}
void FileManager::slot_on_search_result(FileInfo fi, SearchItem search_item)
{
    qRegisterMetaType<FileInfo>("FileInfo");
    qRegisterMetaType<SearchItem>("SearchItem");
    emit signal_search_result(fi, search_item);
}
void FileManager::slot_on_search_request(SearchItem search_item)
{

/*    if(!m_isFileListLoaded) return; // file tree is not ready

    SearchManager* sm = new SearchManager(this, search_item);

    qRegisterMetaType<FileInfo>("FileInfo");
    qRegisterMetaType<SearchItem>("SearchItem");

    connect(sm, SIGNAL(signal_search_result(FileInfo,SearchItem)), this, SLOT(slot_on_search_result(FileInfo,SearchItem)));
    QThreadPool::globalInstance()->start(sm);
    */
}





