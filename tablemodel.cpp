#include "tablemodel.h"
#include <math.h>

TableModel::TableModel(QList<UserInfo>* _list, QObject* pobj) : QAbstractTableModel(pobj)
{
    list = _list;
}

QVariant TableModel::data(const QModelIndex& index, int nRole) const
{
    QString str;
    if(!index.isValid()) {
        return QVariant();
    }
    switch(index.column())
    {
        case 0: // ник
            str = list->at(index.row()).username;
            break;
        case 1: // Шара
            str = QString("%1 Gb").arg((double)(list->at(index.row()).sharesize / powl(1024,3)), 0, 'f', 2);
            break;
        case 2: // Описание
            str = list->at(index.row()).description;
            break;
        case 3: // Client
            str = list->at(index.row()).client;
            break;
    }
    return (nRole == Qt::DisplayRole || nRole == Qt::EditRole) ? m_hash.value(index, QVariant(str)) : QVariant();
}
int TableModel::rowCount(const QModelIndex&) const
{
    return list->size();
}
int TableModel::columnCount(const QModelIndex&) const
{
    return 4;
}
QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal) {
         switch (section) {
             case 0:
                 return tr("Nick");
             case 1:
                 return tr("Share size");
             case 2:
                 return tr("Description");
             case 3:
                 return tr("Client");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }
