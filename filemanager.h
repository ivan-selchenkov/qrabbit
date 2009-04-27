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
    QList<DirsTree> tree;
    QList<QDir> folders;
    void ScanFiles();
    void scan(DirsTree&);
    void SaveXML();
    QDomElement fileDOM(QDomDocument&, QString);
};

#endif // FILEMANAGER_H
