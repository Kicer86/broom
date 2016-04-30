/*
 *    interface for backends
 *    Copyright (C) 2013  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef IBACKEND_HPP
#define IBACKEND_HPP

#include <string>
#include <set>
#include <deque>

#include <core/tag.hpp>
#include <database/photo_data.hpp>

#include "action.hpp"
#include "database_status.hpp"
#include "filter.hpp"

#include "database_export.h"

struct ILoggerFactory;
struct IConfiguration;

namespace Database
{
    struct ProjectInfo;

    //Low level database interface.
    //To be used by particular database backend
    struct DATABASE_EXPORT IBackend
    {
        virtual ~IBackend();

        //add photo to database
        virtual bool addPhoto(Photo::Data &) = 0;

        //update data
        virtual bool update(const Photo::Data &) = 0;
        virtual bool update(const TagNameInfo &) = 0;

        //read data
        virtual std::deque<TagNameInfo> listTags() = 0;                                        // list all stored tag names
        virtual std::deque<QVariant> listTagValues(const TagNameInfo &) = 0;                   // list all values of provided tag
        virtual std::deque<QVariant> listTagValues(const TagNameInfo &,
                                                   const std::deque<IFilter::Ptr> &) = 0;      // list all values for provided tag used on photos matching provided filter
        virtual std::deque<Photo::Id> getAllPhotos() = 0;                                      // list all photos
        virtual std::deque<Photo::Id> getPhotos(const std::deque<IFilter::Ptr> &) = 0;         // find all photos matching filter
        virtual std::deque<Photo::Id> dropPhotos(const std::deque<IFilter::Ptr> &) = 0;        // drop photos matching filter
        virtual Photo::Data           getPhoto(const Photo::Id &) = 0;                         // get particular photo
        virtual int                   getPhotosCount(const std::deque<IFilter::Ptr> &) = 0;    // is there any photo matching filters?
        
        // modify data
        virtual void perform(const std::deque<Database::IFilter::Ptr> &, const std::deque<Database::IAction::Ptr> &) = 0;

        //init backend - connect to database or create new one
        virtual BackendStatus init(const ProjectInfo &) = 0;

        //configuration
        virtual void set(IConfiguration *) = 0;
        virtual void set(ILoggerFactory *) = 0;

        //close database connection
        virtual void closeConnections() = 0;
    };
}

#endif

