/*
 * Model for particular tag info values
 * Copyright (C) 2016  Michał Walenciak <MichalWalenciak@gmail.com>
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

#ifndef TAGVALUEMODEL_HPP
#define TAGVALUEMODEL_HPP

#include <QAbstractItemModel>

class TagValueModel: public QAbstractItemModel
{
public:
    TagValueModel();
    TagValueModel(const TagValueModel &) = delete;
    ~TagValueModel();
    TagValueModel& operator=(const TagValueModel &) = delete;
};

#endif // TAGVALUEMODEL_HPP
