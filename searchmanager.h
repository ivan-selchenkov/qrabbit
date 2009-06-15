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
    SearchManager(QObject* parent, QList<DirsTree> &, QString search, QString mark);
    ~SearchManager();
    //! Searching files & directories
    void run();
    //! Searching in subdirectories
    void searchDirectory(DirsTree &realTree);

private:
    QList<DirsTree> & tree;
    QStringList list;
    QString mark;
    QList<FileInfo> sendlist;
signals:
    void signal_search_finished(QString mark);
    void signal_search_result(FileInfo itemslist, QString mark);
};

#endif // SEARCHMANAGER_H
