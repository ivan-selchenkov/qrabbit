#include "searchmanager.h"

SearchManager::SearchManager(QObject* parent, QList<DirsTree> & _t, QString search, QString _mark): QObject(parent), tree(_t)
{
    list = search.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    mark = _mark;
    qDebug() << "SearchManager()";
}
SearchManager::~SearchManager()
{
    qDebug() << "~SearchManager()";
}
void SearchManager::run()
{
    qDebug() << "SearchManager->run()";
    int i;

    for(i=0; i<tree.size(); i++)
        searchDirectory(tree[i]); // searching in subdirectories

    emit signal_search_finished(mark);
}
void SearchManager::searchDirectory(DirsTree &realTree)
{
    QString cur;

    for(int i=0; i<realTree.childDirs.size(); i++)
        searchDirectory(realTree.childDirs[i]); // searching in subdirectories

    for(int i=0; i<realTree.files.size(); i++) // analyzing file
    {
        foreach(cur, list) // going throught search list
        {
            if(realTree.files[i].filename.contains(cur, Qt::CaseInsensitive)) // if filename contains keyword
            {
                //sendlist.append(realTree.files[i]);
                emit signal_search_result(realTree.files[i], mark);
                break; // file added, ignoring next keywords
            }
        }
    }
}
