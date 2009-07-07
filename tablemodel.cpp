#include "tablemodel.h"
#include <math.h>

TableModel::TableModel(NicklistThreadControl* nlc, QObject* pobj) : QAbstractTableModel(pobj), nicklistControl(nlc)
{
}

QVariant TableModel::data(const QModelIndex& index, int nRole) const
{
    QString str;
    if(!index.isValid()) {
        return QVariant();
    }
    UserInfo user = nicklistControl->at(index.row());
    switch(index.column())
    {
        case 0: // ник
            str = user.username;
            break;
        case 1: // Шара
            str = QString("%1 Gb").arg((double)(user.sharesize / powl(1024,3)), 0, 'f', 2);
            break;
        case 2: // Описание
            str = user.description;
            break;
        case 3: // Client
            str = user.client;
            break;
    }
    return (nRole == Qt::DisplayRole || nRole == Qt::EditRole) ? m_hash.value(index, QVariant(str)) : QVariant();
}
int TableModel::rowCount(const QModelIndex&) const
{
    return nicklistControl->size();
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
