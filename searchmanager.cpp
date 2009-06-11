#include "searchmanager.h"

SearchManager::SearchManager(QObject* parent, QList<DirsTree> & _t, SearchItem & _si): QThread(parent), tree(_t), si(_si)
{
}
void SearchManager::run()
{
    int i;

    for(i=0; i<tree.size(); i++)
        searchDirectory(tree[i]); // searching in subdirectories

    emit signal_search_finished(sendlist, si.mark);
}
void SearchManager::searchDirectory(DirsTree &realTree)
{
    QString cur;

    for(int i=0; i<realTree.childDirs.size(); i++)
        searchDirectory(realTree.childDirs[i]); // searching in subdirectories

    for(int i=0; i<realTree.files.size(); i++) // analyzing file
    {
        foreach(cur, si.list) // going throught search list
        {
            if(realTree.files[i].filename.contains(cur, Qt::CaseInsensitive)) // if filename contains keyword
            {
                sendlist.append(realTree.files[i]);
                break; // file added, ignoring next keywords
            }
        }
    }
}
