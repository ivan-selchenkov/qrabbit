#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QThread>
#include <QtCore>
#include "dirstree.h"
#include "searchitem.h"
#include "fileinfo.h"

class SearchManager : public QThread
{
    Q_OBJECT
public:
    SearchManager(QObject* parent, QList<DirsTree> &, SearchItem&);
    //! Searching files & directories
    void run();
    //! Searching in subdirectories
    void searchDirectory(DirsTree &realTree);

private:
    QList<DirsTree> & tree;
    SearchItem & si;
    QList<FileInfo> sendlist;
signals:
    void signal_search_finished(QList<FileInfo> itemslist, QString mark);
};

#endif // SEARCHMANAGER_H
