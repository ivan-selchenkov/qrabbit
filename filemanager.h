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
    bool operator== (const FileInfo &other ) const {
        if(other.dir == dir && other.filename == filename && other.size == size)
            return true;
        else
            return false;
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
    bool operator== (const DirsTree &other ) const {
        if(other.current == current)
            return true;
        else
            return false;
    }
};
class FileManager: public QObject
{
    Q_OBJECT
public:
    FileManager(QObject* parent);
private:
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

    //! Calculating TTH
    void calculateTTH();
    //! Calculate DirsTree
    void calculateTTHDirectory(DirsTree & realTree);
signals:
    void newInfo(QString);
};

#endif // FILEMANAGER_H
