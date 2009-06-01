#ifndef DIRSTREE_H
#define DIRSTREE_H
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
#endif // DIRSTREE_H
