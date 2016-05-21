/*
 * Interface for PhotoInfo
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

#ifndef IPHOTO_INFO_HPP
#define IPHOTO_INFO_HPP

#include <memory>
#include <string>
#include <deque>

#include <core/tag.hpp>

#include <OpenLibrary/putils/ts_resource.hpp>

#include "database_export.h"

#include "photo_data.hpp"

class QString;
class QImage;

class TagDataBase;

struct DATABASE_EXPORT IPhotoInfo
{
    typedef std::shared_ptr<IPhotoInfo> Ptr;
    typedef std::deque<IPhotoInfo::Ptr> List;

    struct DATABASE_EXPORT IObserver
    {
        virtual ~IObserver();
        virtual void photoUpdated(IPhotoInfo *) = 0;
    };

    virtual ~IPhotoInfo();

    //data getting
    virtual Photo::Data            data() const = 0;
    virtual const QString          getPath() const = 0;
    virtual const Tag::TagsList    getTags() const = 0;       // access to tags
    virtual const QSize            getGeometry() const = 0;   // get photo geometry
    virtual const Photo::Sha256sum getSha256() const = 0;     // Do not call until isSha256Loaded()
    virtual Photo::Id              getID() const = 0;

    //status checking
    virtual bool isFullyInitialized() const = 0;            // returns true if photo fully loaded (all items below are loaded)
    virtual bool isSha256Loaded() const = 0;                // returns true if sha256 is not null
    virtual bool isGeometryLoaded() const = 0;              // returns true if geometry is not null
    virtual bool isExifDataLoaded() const = 0;              // returns true is tags were loaded

    //observers
    virtual void registerObserver(IObserver *) = 0;
    virtual void unregisterObserver(IObserver *) = 0;

    //setting data
    virtual void setSha256(const Photo::Sha256sum &) = 0;
    virtual void setTags(const Tag::TagsList &) = 0;                  // set tags
    virtual void setTag(const TagNameInfo &, const TagValue &) = 0;   // set tag
    virtual void setGeometry(const QSize &) = 0;

    //flags
    virtual void markFlag(Photo::FlagsE, int) = 0;
    virtual int  getFlag(Photo::FlagsE) const = 0;

    // other
    virtual void invalidate() = 0;                          // mark photo as dropped (with no equivalent in db)
    virtual bool isValid() = 0;
};

Q_DECLARE_METATYPE(IPhotoInfo::Ptr)

#endif // IPHOTO_INFO_HPP
