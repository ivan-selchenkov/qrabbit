#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include <QtCore>

struct SearchItem
{
    QStringList list;
    QString mark;
    SearchItem(QString search, QString _mark): mark(_mark)
    {
        list = search.split(QRegExp("\\W+"), QString::SkipEmptyParts);        
    }
};
#endif // SEARCHITEM_H
