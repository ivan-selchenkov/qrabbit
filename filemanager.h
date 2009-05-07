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
    void Clear()
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
};
class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager();
private:
    QDomDocument loadedDoc;
    QList<DirsTree> tree;
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
    void traverseNode(const QDomNode& node, DirsTree & dtree, int level);
    void LoadXML();
};

#endif // FILEMANAGER_H
