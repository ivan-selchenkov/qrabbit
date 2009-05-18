#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QtCore>
#include <QtXml>

struct FileInfo
{
    QString TTH;
    QByteArray interleaves;
    QDir dir;
    QString filename;
    quint64 size;
    void clear()
    {
        TTH.clear();
        interleaves.clear();
        size = 0;
    }
};
struct DirsTree;
struct DirsTree {
    QDir current;
    QList<DirsTree> childDirs;
    QList<FileInfo> files;
    void clear()
    {
        childDirs.clear();
        files.clear();
    }
};
class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager();
private:
    QDomDocument loadedDoc;
    QList<DirsTree> tree;
    QList<DirsTree> loadedTree;
    QList<QDir> folders;
    void ScanFiles();
    void scan(DirsTree&);
    void SaveXML();
    //! Making QList of directories
    QList<QDomElement> makeDOM(QDomDocument&);
    //! Recursive function use for fill QDomElement el
    void nodeDOM(QDomElement& el, QDomDocument& doc, DirsTree & dtree);
    QDomElement fileDOM(QDomDocument&, FileInfo&);
    QDomElement dirDOM(QDomDocument&, QString);
    QDomElement rootDirDOM(QDomDocument&, QString name, QString path);

    //! Finding RootDirectory tags
    void rootDirParse(const QDomNode& node);
    void traverseNode(const QDomNode& node, DirsTree & dtree);
    void LoadXML();
};

#endif // FILEMANAGER_H
