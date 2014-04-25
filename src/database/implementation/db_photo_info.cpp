/*
 * Implementation of APhotoInfo for DataBase
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

#include "db_photo_info.hpp"

#include <assert.h>

#include <core/tag.hpp>

#include "photo_iterator.hpp"


DBPhotoInfo::DBPhotoInfo(const APhotoInfoInitData& data): APhotoInfo(data)
{

}


DBPhotoInfo::~DBPhotoInfo()
{

}


const QPixmap& DBPhotoInfo::getThumbnail() const
{
    assert(!"not implemented");
}
