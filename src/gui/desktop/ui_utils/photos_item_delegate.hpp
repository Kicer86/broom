/*
 * ConfigurableTreeItemDelegate - extension of TreeItemDelegate
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

#ifndef PHOTOSITEMDELEGATE_HPP
#define PHOTOSITEMDELEGATE_HPP

#include <configuration/iconfiguration.hpp>

#include "desktop/views/tree_item_delegate.hpp"


class PhotosItemDelegate: public TreeItemDelegate, private IConfigObserver
{
        Q_OBJECT

    public:
        PhotosItemDelegate(ImagesTreeView *, IConfiguration * = nullptr);
        PhotosItemDelegate(const PhotosItemDelegate &) = delete;
        ~PhotosItemDelegate();

        PhotosItemDelegate& operator=(const PhotosItemDelegate &) = delete;

        void set(IConfiguration *);

        // TreeItemDelegate:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        QImage getImage(const QModelIndex &, const QSize &) const override;

    private:
        IConfiguration* m_config;

        void readConfig();

        void setupEvenColor(const QVariant &);
        void setupOddColor(const QVariant &);

        // IConfigObserver:
        void configChanged(const QString&, const QVariant&) override;
};

#endif // PHOTOSITEMDELEGATE_HPP
