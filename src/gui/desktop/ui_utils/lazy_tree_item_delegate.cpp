/*
 * LazyTreeItemDelegate - loads images from external source
 * Copyright (C) 2015  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "lazy_tree_item_delegate.hpp"

#include <QPainter>

#include <core/down_cast.hpp>

#include "models/aphoto_info_model.hpp"
#include "utils/ithumbnail_acquisitor.hpp"


LazyTreeItemDelegate::LazyTreeItemDelegate(ImagesTreeView* view):
    TreeItemDelegate(view),
    m_thumbnailAcquisitor(),
    m_seriesText("series")
{

}


LazyTreeItemDelegate::~LazyTreeItemDelegate()
{

}


void LazyTreeItemDelegate::set(IThumbnailAcquisitor* acquisitor)
{
    m_thumbnailAcquisitor = acquisitor;
}


QImage LazyTreeItemDelegate::getImage(const QModelIndex& idx, const QSize& size) const
{
    const QAbstractItemModel* model = idx.model();
    const APhotoInfoModel* photoInfoModel = down_cast<const APhotoInfoModel*>(model);      // TODO: not nice (see issue #177)
    const APhotoInfoModel::PhotoDetails details = photoInfoModel->getPhotoDetails(idx);

    const ThumbnailInfo info = {details.path, size.height()};
    QImage image = m_thumbnailAcquisitor->getThumbnail(info);

    if (details.groupInfo.role == GroupInfo::Representative)
    {
        QPainter painter(&image);
        painter.drawStaticText(0, 0, m_seriesText);
    }

    return image;
}
