/*
 * Copyright (C) 2020  Michał Walenciak <Kicer86@gmail.com>
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
 */

#ifndef FLATMODEL_HPP
#define FLATMODEL_HPP

#include "aphoto_info_model.hpp"

namespace Database
{
    struct IDatabase;
}

class FlatModel : public APhotoInfoModel
{
    public:
        FlatModel(QObject* = nullptr);

        void setDatabase(Database::IDatabase *);

        virtual const Photo::Data& getPhotoDetails(const QModelIndex& ) const = 0;

        virtual QVariant data(const QModelIndex& index, int role) const = 0;
        virtual int columnCount(const QModelIndex& parent) const = 0;
        virtual int rowCount(const QModelIndex& parent) const = 0;
        virtual QModelIndex parent(const QModelIndex& child) const = 0;
        virtual QModelIndex index(int row, int column, const QModelIndex& parent) const = 0;

    private:
        Database::IDatabase* m_db;
};

#endif // FLATMODEL_HPP