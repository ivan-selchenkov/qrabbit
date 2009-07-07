#include "searchmanager.h"

SearchManager::SearchManager()
{
}
SearchManager::~SearchManager()
{
    qDebug() << "~SearchManager()";
}
void SearchManager::dbConnect(QSqlDatabase& db)
{
    db = QSqlDatabase::database("SearchThreadControl", true);

    if(!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "SearchThreadControl");

        db.setDatabaseName("files.sqlite");
        if(!db.open())
            qDebug() << "Error opening database in SearchThreadControl";
    }
}

void SearchManager::slot_search(SearchItem si)
{
    // Searching directories
    QSqlDatabase db = QSqlDatabase::database("SearchThreadControl", true);
    if(!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "SearchThreadControl");

        db.setDatabaseName("files.sqlite");
        if(!db.open())
            qDebug() << "Error opening database in SearchThreadControl";
    }

    QSqlQuery query(db);
    QString tth = si.data;

    switch(si.type)
    {
    case SearchItem::TTH:
        tth.remove(0,4);

        query.clear();
        query.prepare("SELECT files.filename, files.filesize, directories.pathRel FROM files INNER JOIN directories ON files.directory_id = directories.id WHERE files.TTH = :tth");
        query.bindValue(":tth", tth);

        if(!query.exec()) {
            qDebug() << "ERROR SearchManager::slot_search():" << query.lastError().driverText() << query.lastError().databaseText() << query.executedQuery();
            return;
        }
        while(query.next())
        {
            FileInfo fi;
            QDir dir = query.value(2).toString();
            fi.relativePath = dir.filePath(query.value(0).toString());
            fi.size = query.value(1).toULongLong();
            fi.TTH = tth;
            fi.isDir = false;
            emit signal_search_result(fi, si);
        }
        break;
    case SearchItem::FOLDER:
        searchFolder(si);
        break;
    case SearchItem::AUDIO:
    case SearchItem::ARCHIVE:
    case SearchItem::BIN:
    case SearchItem::DOCUMENT:
    case SearchItem::IMAGE:
    case SearchItem::ISO:
    case SearchItem::VIDEO:
        searchFiles(si);
        break;
    case SearchItem::ANY:
        searchFolder(si);
        searchFiles(si);
        break;
    }
}
void SearchManager::searchFiles(SearchItem& si)
{
    QSqlDatabase db = QSqlDatabase::database("SearchThreadControl", true);
    if(!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "SearchThreadControl");

        db.setDatabaseName("files.sqlite");
        if(!db.open())
            qDebug() << "Error opening database in SearchThreadControl";
    }

    QSqlQuery query(db);
    QString str = si.data.toUpper();
    QStringList list = str.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    str = "SELECT files.filename, files.filesize, files.TTH, directories.pathRel FROM files INNER JOIN directories ON files.directory_id = directories.id WHERE (";

    for(int i=0; i<list.size(); i++)
        str.append(QString("files.filename_up LIKE :exp%1 OR ").arg(i));

    str.remove(-4,4);

    str.append(") AND files.type = :type");

    query.prepare(str);

    for(int i=0; i < list.size(); i++)
        query.bindValue(QString(":exp%1").arg(i), QString("%%1%").arg(list.at(i)));

    query.bindValue(":type", si.type);

    if(!query.exec()) {
        qDebug() << "ERROR SearchManager::slot_search():" << query.lastError().driverText() << query.lastError().databaseText() << query.executedQuery();
        return;
    }
    while(query.next())
    {
        FileInfo fi;
        QDir dir = query.value(3).toString();
        fi.relativePath = dir.filePath(query.value(0).toString());
        fi.size = query.value(1).toULongLong();
        fi.TTH = query.value(2).toString();
        fi.isDir = false;
        emit signal_search_result(fi, si);
    }
}

void SearchManager::searchFolder(SearchItem & si)
{
    QSqlDatabase db = QSqlDatabase::database("SearchThreadControl", true);
    if(!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QSQLITE", "SearchThreadControl");

        db.setDatabaseName("files.sqlite");
        if(!db.open())
            qDebug() << "Error opening database in SearchThreadControl";
    }

    QSqlQuery query(db);
    QString str = si.data.toUpper();
    QStringList list = str.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    str = "SELECT pathRel FROM directories WHERE ";
    for(int i=0; i<list.size(); i++)
        str.append(QString("name_up LIKE :exp%1 OR ").arg(i));

    str.remove(-4,4);

    query.prepare(str);

    for(int i=0; i < list.size(); i++)
        query.bindValue(QString(":exp%1").arg(i), QString("%%1%").arg(list.at(i)));


    if(!query.exec()) {
        qDebug() << "ERROR SearchManager::slot_search():" << query.lastError().driverText() << query.lastError().databaseText() << query.executedQuery();
        return;
    }
    while(query.next())
    {
        FileInfo fi;
        fi.relativePath = query.value(0).toString();
        fi.isDir = true;
        emit signal_search_result(fi, si);
    }

}
