/*
 * Base for models offering scalable decoration role.
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

#ifndef ASCALABLEIMAGESMODEL_HPP
#define ASCALABLEIMAGESMODEL_HPP

#include <QAbstractItemModel>
#include <QCache>
#include <QImage>

#include <core/callback_ptr.hpp>

struct ITaskExecutor;

class APhotoInfoModel: public QAbstractItemModel
{
    public:
        struct Key
        {
            QModelIndex index;
            QSize size;

            Key(const QModelIndex& k, const QSize& s): index(k), size(s) {}
            Key(const Key &) = default;

            Key& operator=(const Key &) = default;
            bool operator==(const Key& other) const
            {
                return index == other.index &&
                       size == other.size;
            }
        };

        APhotoInfoModel(QObject * = 0);
        APhotoInfoModel(const APhotoInfoModel &) = delete;
        ~APhotoInfoModel();

        APhotoInfoModel& operator=(const APhotoInfoModel &) = delete;

        QImage getDecorationRole(const QModelIndex &, const QSize &);

        void set(ITaskExecutor *);

    protected:
        // will be called from non-main thread
        virtual QImage getImageFor(const QModelIndex &, const QSize &) = 0;

    private:
        QCache<Key, QImage> m_cache;
        safe_callback_ctrl m_callback_ctrl;
        ITaskExecutor* m_taskExecutor;

        void loadImage(const Key &);
};

uint qHash(const APhotoInfoModel::Key& key);

#endif // ASCALABLEIMAGESMODEL_HPP
