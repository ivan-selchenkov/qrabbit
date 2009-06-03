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
    quint64 totalCount;
    QList<DirsTree> * tree;
    //! Calculate DirsTree
    void calcDirectory(DirsTree & realTree);

signals:
    void newInfo(QString);
    void progressStatus(quint64);
    void finished();
    void saveTree(bool);
};

#endif // CALCULATETTH_H
