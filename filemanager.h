#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QtCore>
#include <QtXml>
#include "fileinfo.h"
#include "dirstree.h"
#include "calculatetth.h"

class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager(QObject* parent);
private:
    CalculateTTH* ctth;
    QTime time;
    QDomDocument loadedDoc;
    QList<DirsTree> tree;
    QList<DirsTree> loadedTree;
    QList<QDir> folders;
    void scanFiles();
    void scan(DirsTree&);
    void saveXML();
    //! Making QList of directories
    QList<QDomElement> makeDOM(QDomDocument&);
    //! Recursive function use for fill QDomElement el
    void nodeDOM(QDomElement& el, QDomDocument& doc, DirsTree & dtree);
    QDomElement fileDOM(QDomDocument&, FileInfo&);
    QDomElement dirDOM(QDomDocument&, QString);
    QDomElement rootDirDOM(QDomDocument&, QString name, QString path);

    //! Finding RootDirectory tags
    void rootDirParse(const QDomNode& node);
    //! Parsing child directories
    void traverseNode(const QDomNode& node, DirsTree & dtree);
    //! Loading XML Filelist
    void loadXML();

    //! Setting TTH value for early hashed files
    void setTTH();
    //! Recursive function to set TTH in directory
    void setTTHDirectory(DirsTree & loadedTree, DirsTree & realTree);
public slots:
    void slotSaveXML(bool);
signals:
    void newInfo(QString);
};
#endif // FILEMANAGER_H
