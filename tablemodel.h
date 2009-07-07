#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QtGui>
#include "nicklistthreadcontrol.h"

struct UserInfo;

class TableModel : public QAbstractTableModel {
private:
    QHash<QModelIndex, QVariant> m_hash;
    NicklistThreadControl* nicklistControl;
public:
    TableModel(NicklistThreadControl*, QObject* pobj = 0);
    QVariant data(const QModelIndex& index, int nRole) const;
    int rowCount(const QModelIndex&) const;
    int columnCount(const QModelIndex&) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // TABLEMODEL_H
