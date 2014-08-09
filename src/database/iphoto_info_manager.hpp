/*
 * Interface for PhotoInfoManager
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

#ifndef IPHOTO_INFO_MANAGER_HPP
#define IPHOTO_INFO_MANAGER_HPP

#include <core/photo_info.hpp>

struct IPhotoInfoManager
{
    virtual ~IPhotoInfoManager() {}

    virtual PhotoInfo::Ptr find(const PhotoInfo::Id &) const = 0;
    virtual void introduce(const PhotoInfo::Ptr &) = 0;
};


#endif
