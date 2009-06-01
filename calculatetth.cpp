#include "calculatetth.h"

CalculateTTH::CalculateTTH(QObject* parent, QList<DirsTree> * _t): QThread(parent), tree(_t)
{    
}
void CalculateTTH::run()
{
    int index;

    for(index=0; index<tree->size(); index++)
        calcDirectory((*tree)[index]);

    emit saveTree(true);
}
void CalculateTTH::calcDirectory(DirsTree & realTree)
{
    HashFile hf;
    FileInfo file;
    int index;

    for(index=0; index<realTree.childDirs.size(); index++)
        calcDirectory(realTree.childDirs[index]);

    for(index=0; index<realTree.files.size(); index++)
    {
        if(realTree.files[index].TTH.isEmpty())
        {
            emit newInfo(QFileInfo(realTree.files[index].dir, realTree.files[index].filename).absoluteFilePath());
            realTree.files[index].TTH = hf.Go(QFileInfo(realTree.files[index].dir, realTree.files[index].filename).absoluteFilePath());
        }
        emit saveTree(false);
    }
}
