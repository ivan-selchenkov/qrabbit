#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QtCore>
#include <QtXml>
#include "fileinfo.h"
#include "dirstree.h"
#include "calculatetth.h"
#include "searchitem.h"

class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager(QObject* parent);
    ~FileManager();
private:
    quint64 totalCount;
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
    
    //! Searching files & directories
    void searchStart(SearchItem& search);
    //! Searching in subdirectories
    void searchDirectory(SearchItem&, DirsTree &realTree);

public slots:
    void slotSaveXML(bool);
    void onHasingStatus(quint64);
    void on_search_request(QString search, QString mark);
    void on_search_finished(QList<FileInfo> list, QString mark);
signals:
    //! Signal for MainWindow - new info for status bar
    void newInfo(QString);
    //! Signal for MainWindow - hashing progress
    void progressInfo(int);
    void searchFinished(QList<FileInfo> itemslist, QString mark);
};
#endif // FILEMANAGER_H
