
#include "notifications_model.hpp"

void NotificationsModel::insertWarning(const QString& warning)
{
    const int count = getCount();

    beginInsertRows({}, count, count);
    m_data.append(warning);
    endInsertRows();

    emit countChanged(getCount());
}


int NotificationsModel::getCount() const
{
    return m_data.size();
}


int NotificationsModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid()? 0: getCount();
}

QVariant NotificationsModel::data(const QModelIndex& index, int role) const
{
    return role == Qt::DisplayRole? m_data[index.row()]: QVariant();
}