#ifndef FILEINFO_H
#define FILEINFO_H
struct FileInfo
{
    QString TTH;
    QByteArray interleaves;
    QDir dir;
    QDir relativeDir;
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
#endif // FILEINFO_H
