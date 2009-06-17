#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QThread>
#include <QtCore>
#include <QRunnable>
#include "dirstree.h"
#include "searchitem.h"
#include "fileinfo.h"

class SearchManager : public QObject, public QRunnable //public QThread
{
    Q_OBJECT
public:
    SearchManager(QObject* parent, QList<DirsTree> &, SearchItem);
    ~SearchManager();
    //! Searching files & directories
    void run();
    //! Searching in subdirectories
    void searchDirectory(DirsTree &realTree);

private:
    QList<DirsTree> & tree;
    QStringList list;
    QRegExp exp;
    SearchItem search_item;
    QList<FileInfo> sendlist;
signals:
    void signal_search_result(FileInfo itemslist, SearchItem si);
};

#endif // SEARCHMANAGER_H
