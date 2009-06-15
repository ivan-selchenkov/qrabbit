#ifndef INITFILETREE_H
#define INITFILETREE_H

#include <QThread>
#include <QtCore>
#include <QtXml>
#include "fileinfo.h"
#include "dirstree.h"
#include "tth/hashfile.h"

class InitFileTree : public QThread
{
    Q_OBJECT
public:
    InitFileTree(QObject* parent, QList<DirsTree> &, QList<QDir>& folders);
    ~InitFileTree();
    void run();
private:
    quint64 totalCount;
    quint64 hashedCount;

    QList<DirsTree> & tree;
    QList<QDir>& folders;
    QList<DirsTree> loadedTree;

    QTime time;
    QDomDocument loadedDoc;

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

    void scanFiles();
    void scan(DirsTree&, const QDir& top);
    void saveXML();

    void slotSaveXML(bool);

    void calcTTH();
    void calcDirectory(DirsTree & realTree);
//! Making QList of directories
    QList<QDomElement> makeDOM(QDomDocument&);
    //! Recursive function use for fill QDomElement el
    void nodeDOM(QDomElement& el, QDomDocument& doc, DirsTree & dtree);
    QDomElement fileDOM(QDomDocument&, FileInfo&);
    QDomElement dirDOM(QDomDocument&, QString);
    QDomElement rootDirDOM(QDomDocument&, QString name, QString path);

 signals:
    void signal_finished();
    void signal_hashing_progress(int);

};

#endif // INITFILETREE_H
