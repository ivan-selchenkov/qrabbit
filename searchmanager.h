#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QThread>
#include <QtCore>
#include <QtSql>
#include "searchitem.h"
#include "fileinfo.h"

class SearchManager : public QThread
{
    Q_OBJECT
public:
    SearchManager();
    ~SearchManager();   

private:
    void dbConnect(QSqlDatabase&);
    void searchFolder(SearchItem&);
    void searchFiles(SearchItem&);
public slots:
    void slot_search(SearchItem);
signals:
    void signal_search_result(FileInfo itemslist, SearchItem si);
};

#endif // SEARCHMANAGER_H
