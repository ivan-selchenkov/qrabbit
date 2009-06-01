#ifndef CALCULATETTH_H
#define CALCULATETTH_H

#include <QThread>
#include <QtCore>
#include "fileinfo.h"
#include "dirstree.h"
#include "tth/hashfile.h"

class CalculateTTH: public QThread
{
    Q_OBJECT
public:
    CalculateTTH(QObject* parent, QList<DirsTree> *);
    void run();
private:
    QList<DirsTree> * tree;
    //! Calculate DirsTree
    void calcDirectory(DirsTree & realTree);

signals:
    void newInfo(QString);
    void saveTree(bool);
};

#endif // CALCULATETTH_H
