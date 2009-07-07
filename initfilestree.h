#ifndef INITFILESTREE_H
#define INITFILESTREE_H

#include <QThread>
#include <QtCore>
#include <QtSql>
#include <QtXml>

class InitFilesTree : public QThread
{
    Q_OBJECT
public:
    InitFilesTree(QObject* parent, QList<QString> folders);
    ~InitFilesTree();
    void run();
private:
    quint64 totalCount;
    quint64 hashedCount;

    QStringList folders;

    void scan();
    void scanFolder(QDir& parent, QDir& top, int parent_id);
    void calculateTTH();
    int getType(QString);

    void saveXML();
    //! Recursive function use for fill QDomElement el
    void nodeDOM(QDomElement& el, QDomDocument& doc, int parent_id);
    QDomElement fileDOM(QDomDocument&, QString filename, QString TTH, quint64 size);
    QDomElement dirDOM(QDomDocument&, QString);
    void bz2Compress();

    QSqlDatabase dbConnect();
    void execQuery(QSqlQuery&);

signals:
    void signal_finished();
    void signal_hashing_progress(int);
    void signal_new_sharesize(quint64);
};

#endif // INITFILESTREE_H
