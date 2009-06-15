#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include <QtCore>

struct SearchItem
{
    QStringList list;
    QString mark;
    SearchItem(QString search, QString _mark): mark(_mark)
    {
    }
    SearchItem()
    {
    }
};
#endif // SEARCHITEM_H
