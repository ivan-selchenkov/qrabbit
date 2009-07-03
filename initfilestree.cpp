#include "initfilestree.h"
#include "tth/hashfile.h"
#include <bzlib.h>

InitFilesTree::InitFilesTree(QObject* parent, QList<QString> _folders): QThread(parent)
{
    folders.append(_folders);

    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isValid()) db = dbConnect();
    // CREATE TABLE "files" ("id" INTEGER PRIMARY KEY  NOT NULL ,"filename" TEXT, "filename_up" TEXT, "TTH" TEXT,"directory_id" INTEGER,"isActual" INTEGER DEFAULT 1 ,"filesize" INTEGER NOT NULL  DEFAULT 0 )
    // CREATE TABLE "directories" ("id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "name" TEXT, "name_up" TEXT, "pathRel" TEXT, "pathAbs" TEXT, "parent_id" INTEGER, "isActual" INTEGER DEFAULT 1)

    // CREATE INDEX "TTH" ON "files" ("TTH" ASC)
    // CREATE INDEX "dirname" ON "directories" ("name_up" ASC)
    // CREATE INDEX "filename" ON "files" ("filename_up" ASC)

    // REINDEX "TTH"
    // REINDEX "dirname"
    // REINDEX "filename"

    QSqlQuery* query = new QSqlQuery("UPDATE files SET isActual = 0", db);
    if(!query->exec()) {
        qDebug() << "ERROR ACTUAL RESET files: " << query->lastError();
    }
    query->clear();
    query->prepare("UPDATE directories SET isActual = 0");
    if(!query->exec()) {
        qDebug() << "ERROR ACTUAL RESET directories: " << query->lastError();
    }
}
InitFilesTree::~InitFilesTree()
{
    QSqlDatabase db = QSqlDatabase::database();
    db.close();
}
void InitFilesTree::run()
{
    quint64 totalShare;

    scan();
    calculateTTH();
    saveXML();

    emit signal_finished();

    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isValid()) db = dbConnect();

    QSqlQuery query = QSqlQuery(db);
    bool ok;

    query.prepare("SELECT SUM(filesize) FROM files WHERE TTH IS NOT NULL");
    if(!query.exec()) {
        qDebug() << query.lastQuery();
        return;
    }
    query.next();
    totalShare = query.value(0).toULongLong(&ok);
    if(ok)
        emit signal_new_sharesize(totalShare);
}
void InitFilesTree::scan()
{
    QDir top;
    QDir current;

    for(int i=0; i<folders.size(); i++)
    {
        if(!QDir(folders.at(i)).exists()) continue;
        current = QDir(folders.at(i));
        top = current;
        top.cdUp();
        scanFolder(current, top, 0);
    }
    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isValid()) db = dbConnect();

    QSqlQuery* query = new QSqlQuery("DELETE FROM files WHERE isActual = 0", db);
    if(!query->exec()) {
        qDebug() << query->lastQuery();
    }
    query->clear();
    query->prepare("DELETE FROM directories WHERE isActual = 0");
    if(!query->exec()) {
        qDebug() << query->lastQuery();
    }
}
void InitFilesTree::calculateTTH()
{
    HashFile hf;
    bool ok;

    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isValid()) db = dbConnect();

    QSqlQuery query = QSqlQuery(db);
    QSqlQuery query_update = QSqlQuery(db);

    query.clear();
    query.prepare("SELECT SUM(filesize) FROM files WHERE TTH IS NULL");
    if(!query.exec()) {
        qDebug() << query.lastQuery();
        return;
    }
    query.next();
    totalCount = query.value(0).toULongLong(&ok);
    if(!ok || totalCount == 0) // where is no files for hash
        return;

    hashedCount = 0;
    //SELECT files.filename, directories.pathAbs FROM files INNER JOIN directories ON files.directory_id = directories.id WHERE files.TTH IS NULL
    query.clear();
    query.prepare("SELECT files.id, files.filename, files.filesize, directories.pathAbs FROM files INNER JOIN directories ON files.directory_id = directories.id WHERE files.TTH IS NULL");
    if(!query.exec()) {
        qDebug() << query.lastQuery();
        return;
    }
    QString filename;
    QString directory;
    QString filedir;
    int id;
    quint64 filesize;
    QString TTH;
    QDir dir;
    while(query.next())
    {
        id = query.value(0).toInt();
        filename = query.value(1).toString();
        filesize = query.value(2).toULongLong();
        directory = query.value(3).toString();
        dir.setPath(directory);
        qDebug() << "Hashing: " << filename;
        filedir = QFileInfo(dir, filename).absoluteFilePath();
        TTH = hf.Go(filedir);
        hashedCount += filesize;

        emit signal_hashing_progress(100 * hashedCount / totalCount);

        query_update.clear();
        query_update.prepare("UPDATE files SET TTH = :TTH WHERE id = :id");
        query_update.bindValue(":TTH", TTH);
        query_update.bindValue(":id", id);

        if(!query_update.exec()) {
            qDebug() << query_update.lastQuery();
            return;
        }
    }
}
void InitFilesTree::scanFolder(QDir& parent /* current directory */, QDir & top /* top direcotry */, int parent_id /* id of current direcotry in db */)
{
    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isValid()) db = dbConnect();

    QSqlQuery query = QSqlQuery(db);

    int next_parent_id;

    // finding directory in db
    query.prepare("SELECT id FROM directories WHERE pathAbs = :path AND parent_id = :id");
    query.bindValue(":path", parent.absolutePath());
    query.bindValue(":id", parent_id);

    if(!query.exec()) {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    // checking is directory in db?
    if(query.next())
    {
        next_parent_id = query.value(0).toInt();
        query.clear();
        query.prepare("UPDATE directories SET isActual = 1 WHERE id = :id");
        query.bindValue(":id", next_parent_id);

        if(!query.exec()) {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
    }
    // else adding it to db
    else
    {
        query.clear();
        query.prepare("INSERT INTO directories(name, name_up, pathRel, pathAbs, parent_id, isActual) VALUES(:name, :name_up, :pathRel, :pathAbs, :parent_id, :isActual)");
        query.bindValue(":name", parent.dirName());
        query.bindValue(":name_up", parent.dirName().toUpper());
        QString rel = top.relativeFilePath(parent.absolutePath());
        query.bindValue(":pathRel", rel);
        query.bindValue(":pathAbs", parent.absolutePath());
        query.bindValue(":parent_id", parent_id);
        query.bindValue(":isActual", 1);

        if(!query.exec()) {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        next_parent_id = query.lastInsertId().toInt();
    }

    /* CREATE TABLE "files" (
    "id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL ,
    "filename" TEXT,
    "TTH" TEXT,
    "directory_id" INTEGER,
    "isActual" INTEGER DEFAULT 1,
    "filesize" INTEGER NOT NULL  DEFAULT 0)
    */
    // ANALYZING FILES IN PARENT DIRECTORY
    QStringList listFiles = parent.entryList(QDir::Files);
    int file_id;

    foreach(QString file, listFiles)
    {
        // finding file in db
        query.prepare("SELECT id FROM files WHERE filename = :filename AND directory_id = :directory_id AND filesize = :filesize");
        query.bindValue(":filename", file);
        query.bindValue(":directory_id", next_parent_id);
        query.bindValue(":filesize", QFileInfo(parent.absoluteFilePath(file)).size());

        if(!query.exec()) {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        // checking is file in db?
        if(query.next())
        {
            file_id = query.value(0).toInt();
            query.clear();
            query.prepare("UPDATE files SET isActual = 1 WHERE id = :id");
            query.bindValue(":id", file_id);

            if(!query.exec()) {
                qDebug() << query.lastError();
                qDebug() << query.lastQuery();
            }
        }
        // else adding it to db
        else
        {
            query.clear();
            query.prepare("INSERT INTO files(filename, filename_up, directory_id, isActual, filesize) VALUES(:filename, :filename_up, :directory_id, :isActual, :filesize)");
            query.bindValue(":filename", file);
            query.bindValue(":filename_up", file.toUpper());
            query.bindValue(":directory_id", next_parent_id);
            query.bindValue(":isActual", 1);
            query.bindValue(":filesize", QFileInfo(parent.absoluteFilePath(file)).size());

            if(!query.exec()) {
                qDebug() << query.lastError();
                qDebug() << query.lastQuery();
            }
        }
    }

    // RECURCING DIRECTORIES
    QStringList listDirs = parent.entryList(QDir::Dirs);
    QDir next;
    foreach(QString subdir, listDirs)
    {
        if(subdir == "." || subdir == "..") continue;
        next = parent;
        next.cd(subdir);
        scanFolder(next, top, next_parent_id);
    }
}
QSqlDatabase InitFilesTree::dbConnect()
{
    bool isExists;
    // CREATE TABLE "files" ("id" INTEGER PRIMARY KEY  NOT NULL ,"filename" TEXT, "filename_up" TEXT, "TTH" TEXT,"directory_id" INTEGER,"isActual" INTEGER DEFAULT 1 ,"filesize" INTEGER NOT NULL  DEFAULT 0 )
    // CREATE TABLE "directories" ("id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "name" TEXT, "name_up" TEXT, "pathRel" TEXT, "pathAbs" TEXT, "parent_id" INTEGER, "isActual" INTEGER DEFAULT 1)

    // CREATE INDEX "TTH" ON "files" ("TTH" ASC)
    // CREATE INDEX "dirname" ON "directories" ("name_up" ASC)
    // CREATE INDEX "filename" ON "files" ("filename_up" ASC)

    if(QFileInfo("files.sqlite").exists())
       isExists = true;
    else
        isExists = false;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("files.sqlite");

    if(!db.open())
    {
        qDebug() << "Error opening database";
        return QSqlDatabase();
    }
    QSqlQuery query;
    if(!isExists)
    {
        query.prepare("CREATE TABLE \"files\" (\"id\" INTEGER PRIMARY KEY  NOT NULL ,\"filename\" TEXT, \"filename_up\" TEXT, \"TTH\" TEXT,\"directory_id\" INTEGER,\"isActual\" INTEGER DEFAULT 1 ,\"filesize\" INTEGER NOT NULL  DEFAULT 0 )");
        execQuery(query);

        query.prepare("CREATE TABLE \"directories\" (\"id\" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , \"name\" TEXT, \"name_up\" TEXT, \"pathRel\" TEXT, \"pathAbs\" TEXT, \"parent_id\" INTEGER, \"isActual\" INTEGER DEFAULT 1)");
        execQuery(query);

        query.prepare("CREATE INDEX \"TTH\" ON \"files\" (\"TTH\" ASC)");
        execQuery(query);

        query.prepare("CREATE INDEX \"dirname\" ON \"directories\" (\"name_up\" ASC)");
        execQuery(query);

        query.prepare("CREATE INDEX \"filename\" ON \"files\" (\"filename_up\" ASC)");
        execQuery(query);
    }
    return db;
}
void InitFilesTree::execQuery(QSqlQuery& query)
{
    if(!query.exec())
        qDebug() << query.lastError() << query.lastQuery();
}
void InitFilesTree::saveXML()
{
    QString xml = "xml";
    QDomDocument doc;

    QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    doc.insertBefore(node, doc.firstChild());

    QDomElement docElement = doc.createElement("FileListing");
    doc.appendChild(docElement);

    nodeDOM(docElement, doc, 0);

    QFile file("files.xml");

    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream(&file) << doc.toString();
        file.close();
    }

    bz2Compress();

//    emit signal_new_sharesize(totalCount);
}
void InitFilesTree::bz2Compress()
{
    FILE*   f;
    BZFILE* b;

    int     bzerror;
    int     nWritten;
    unsigned int n, n1;

    QFile file("files.xml");
    if(!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();

    f = fopen ("files.xml.bz2", "w");
    if(!f) return;

    b = BZ2_bzWriteOpen( &bzerror, f, 9, 0, 30);
    if (bzerror != BZ_OK) {
        BZ2_bzWriteClose (&bzerror, b, 0, &n, &n1 );
        return;
    }

    BZ2_bzWrite ( &bzerror, b, (void *) data.constData(), data.size() );

    BZ2_bzWriteClose( &bzerror, b, 0, &n, &n1 );
}

void InitFilesTree::nodeDOM(QDomElement& el, QDomDocument& doc, int parent_id)
{
    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isValid()) db = dbConnect();

    QSqlQuery query = QSqlQuery(db);

    query.prepare("SELECT id, name FROM directories WHERE parent_id = :id");
    query.bindValue(":id", parent_id);

    if(!query.exec())
        qDebug() << query.lastError() << query.lastQuery();

    QString dirname;
    int id;

    while(query.next())
    {
        id = query.value(0).toInt();
        dirname = query.value(1).toString();
        QDomElement m_el = dirDOM(doc, dirname);
        nodeDOM(m_el, doc, id); // recursing
        el.appendChild(m_el);
    }

    query.clear();
    query.prepare("SELECT filename, TTH, filesize FROM files WHERE directory_id = :id");
    query.bindValue(":id", parent_id);

    if(!query.exec())
        qDebug() << query.lastError() << query.lastQuery();

    QString filename;
    QString TTH;
    quint64 size;

    while(query.next())
    {
        filename = query.value(0).toString();
        TTH = query.value(1).toString();
        size = query.value(2).toULongLong();
        QDomElement m_el = fileDOM(doc, filename, TTH, size);
        if(!TTH.isEmpty())
            el.appendChild(m_el);
    }
}

QDomElement InitFilesTree::dirDOM(QDomDocument& doc, QString str)
{
    QDomElement element = doc.createElement("Directory");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(str);
    element.setAttributeNode(atr);
    return element;
}
QDomElement InitFilesTree::fileDOM(QDomDocument& doc, QString filename, QString TTH, quint64 size)
{
    QDomElement element = doc.createElement("File");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(filename);
    element.setAttributeNode(atr);

    atr = doc.createAttribute("TTH");
    atr.setValue(TTH);
    element.setAttributeNode(atr);

    atr = doc.createAttribute("Size");
    atr.setValue(QString::number(size));
    element.setAttributeNode(atr);

    return element;
}
