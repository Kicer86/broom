/*
 * Tags model
 * Copyright (C) 2014  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/// TODO: remove
#if defined _MSC_VER
    #if _MSC_VER >= 1800
        #define Q_COMPILER_INITIALIZER_LISTS
    #else
        #error unsupported compiler
    #endif
#endif


#include "tags_model.hpp"

#include <QItemSelectionModel>
#include <QDate>
#include <QTime>

#include "model_view/db_data_model.hpp"
#include "tags_operator.hpp"



TagsModel::TagsModel(QObject* p):
    QStandardItemModel(p),
    m_selectionModel(nullptr),
    m_dbDataModel(nullptr),
    m_tagsOperator()
{
}


TagsModel::~TagsModel()
{

}


void TagsModel::set(QItemSelectionModel* selectionModel)
{
    if (m_selectionModel != nullptr)
        m_selectionModel->disconnect(this);

    m_selectionModel = selectionModel;
    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(refreshModel(QItemSelection, const QItemSelection &)));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(updateData(QModelIndex,QModelIndex)));

    refreshModel();
}


void TagsModel::set(DBDataModel* dbDataModel)
{
    m_dbDataModel = dbDataModel;
}


void TagsModel::set(ITagsOperator* tagsOperator)
{
    m_tagsOperator = tagsOperator;
}


Tag::TagsList TagsModel::getTags() const
{
    return m_tagsOperator->getTags();
}


void TagsModel::addTag(const TagNameInfo& info, const QString& value)
{
    m_tagsOperator->setTag(info, TagValue(value));

    refreshModel();
}


void TagsModel::refreshModel()
{
    if (m_dbDataModel != nullptr && m_selectionModel != nullptr)
    {
        clearModel();

        std::vector<IPhotoInfo::Ptr> photos = getPhotosForSelection();
        m_tagsOperator->operateOn(photos);

        Tag::TagsList tags = getTags();

        for (const auto& tag: tags)
        {
            Tag::Info info(tag);
            QStandardItem* name = new QStandardItem(info.displayName());
            QStandardItem* value = new QStandardItem;
            value->setData(getValueFor(info), Qt::EditRole);

            const QList<QStandardItem *> items( { name, value });
            appendRow(items);
        }

        emit modelChanged(photos.empty() == false);
    }
}


void TagsModel::clearModel()
{
    clear();
    setHorizontalHeaderLabels( {tr("Name"), tr("Value")} );
}


std::vector<IPhotoInfo::Ptr> TagsModel::getPhotosForSelection()
{
    std::vector<IPhotoInfo::Ptr> result;

    QItemSelection selection = m_selectionModel->selection();

    for (const QItemSelectionRange& range : selection)
    {
        QModelIndexList idxList = range.indexes();

        for (const QModelIndex& idx : idxList)
        {
            IPhotoInfo::Ptr photo = m_dbDataModel->getPhoto(idx);

            if (photo.get() != nullptr)
                result.push_back(photo);
        }
    }

    return result;
}


QVariant TagsModel::getValueFor(const Tag::Info& i) const
{
    TagNameInfo::Type type = i.getTypeInfo().getType();
    QVariant result;
    const QString value = i.value().get();
    
    switch(type)
    {
        case TagNameInfo::Invalid:
        case TagNameInfo::Text:
            result = value;
            break;
            
        case TagNameInfo::Date:
            result = QDate::fromString(value, "yyyy.MM.dd");
            break;
            
        case TagNameInfo::Time:
            result = QTime::fromString(value, "hh:mm:ss");
            break;
    };
    
    return result;
}


QString TagsModel::getValueFor(const QVariant& v) const
{
    const QVariant::Type type = v.type();
    QString result;

    switch(type)
    {
        default:
            assert(!"unknown type");

        case QVariant::String:
            result = v.toString();
            break;

        case QVariant::Date:
            result = v.toDate().toString("yyyy.MM.dd");
            break;

        case QVariant::Time:
            result = v.toTime().toString("hh::mm::ss");
            break;
    };

    return result;
}


void TagsModel::refreshModel(const QItemSelection &, const QItemSelection &)
{
    refreshModel();
}


void TagsModel::updateData(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    const QItemSelection items(topLeft, bottomRight);
    const QModelIndexList itemsList(items.indexes());

    for (const QModelIndex& itemIndex: itemsList)
    {
        assert(itemIndex.column() == 1);

        if (itemIndex.column() == 1)
        {
            const QModelIndex tagNameIndex = itemIndex.sibling(itemIndex.row(), 0);
            const QString tagName = tagNameIndex.data().toString();
            const QVariant rawValue = itemIndex.data();
            const QString tagValue = getValueFor(rawValue);

            m_tagsOperator->updateTag(tagName, tagValue);
        }
    }
}
