#include "filemanager.h"
#include "mainwindow.h"

FileManager::FileManager(QObject* parent): QObject(parent)
{
    folders.append(QDir("/home/ivan/Downloads"));

    ift = new InitFileTree(this, &tree, &folders);
    ift->start();
    //ctth = new CalculateTTH(this, &tree);

    //connect(ctth, SIGNAL(newInfo(QString)), (MainWindow*) parent, SLOT(on_info(QString)));
    //connect(ctth, SIGNAL(progressStatus(quint64)), this, SLOT(onHasingStatus(quint64)));
    //connect(ctth, SIGNAL(saveTree(bool)), this, SLOT(slotSaveXML(bool)));
    //connect(ctth, SIGNAL(finished()), (MainWindow*) parent, SLOT(on_hashing_finished()));

    connect(this, SIGNAL(searchFinished(QList<FileInfo>,QString)), this, SLOT(on_search_finished(QList<FileInfo>,QString)));

    //on_search_request("Chorus", "xxx");
    //saveXML();
}
void FileManager::initScan()
{
    emit on_init_scan();
}
void FileManager::on_init_scan()
{
}
void FileManager::on_search_finished(QList<FileInfo> itemslist, QString mark)
{
    //qDebug() << itemslist[0].filename;
}

void FileManager::on_search_request(QString search, QString mark)
{
    SearchItem si(search, mark);
    searchStart(si);
}
void FileManager::onHasingStatus(quint64 hashed)
{
    emit progressInfo(100 * hashed/ totalCount);
}
FileManager::~FileManager()
{
    ift->terminate();
    ift->wait();
}




void FileManager::searchStart(SearchItem& search)
{
    int i;

    for(i=0; i<tree.size(); i++)
        searchDirectory(search, tree[i]); // searching in subdirectories
}
void FileManager::searchDirectory(SearchItem& search, DirsTree &realTree)
{
    QString cur;
    QList<FileInfo> sendlist;

    for(int i=0; i<realTree.childDirs.size(); i++)
        searchDirectory(search, realTree.childDirs[i]); // searching in subdirectories

    for(int i=0; i<realTree.files.size(); i++) // analyzing file
    {
        foreach(cur, search.list) // going throught search list
        {
            if(realTree.files[i].filename.contains(cur, Qt::CaseInsensitive)) // if filename contains keyword
            {
                sendlist.append(realTree.files[i]);
                break; // file added, ignoring next keywords
            }
        }
    }
    emit searchFinished(sendlist, search.mark);
}
