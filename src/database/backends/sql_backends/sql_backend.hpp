/*
 * Base for SQL-based backends
 * This class is meant to be included to each project using it.
 *
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

#ifndef ASQLBACKEND_H
#define ASQLBACKEND_H

#include <memory>
#include <vector>
#include <deque>

#include "database/ibackend.hpp"
#include "sql_backend_base_export.h"

class QSqlQuery;
class QSqlDatabase;

struct IPhotoInfoManager;

namespace Database
{

    class Entry;
    class InsertQueryData;
    struct ISqlQueryConstructor;
    struct TableDefinition;

    class SQL_BACKEND_BASE_EXPORT ASqlBackend: public Database::IBackend
    {
        public:
            struct Data;

            ASqlBackend();
            ASqlBackend(const ASqlBackend& other) = delete;
            virtual ~ASqlBackend();

            ASqlBackend& operator=(const ASqlBackend& other) = delete;
            bool operator==(const ASqlBackend& other) = delete;

            void closeConnections();

            const QString& getConnectionName() const;

        protected:
            //will be called from init(). Prepare QSqlDatabase object here
            virtual BackendStatus prepareDB(const ProjectInfo& location) = 0;

            // called when db opened. Backend may perform some extra setup
            virtual bool dbOpened();

            //make sure table exists. Makes sure a table maching TableDefinition exists in database
            virtual BackendStatus assureTableExists(const TableDefinition &) const;

            //execute query. Function for inheriting classes
            virtual bool exec(const QString &, QSqlQuery *) const;

            virtual const ISqlQueryConstructor* getQueryConstructor() const = 0;

            virtual void set(ILoggerFactory *) override;

        private:
            std::unique_ptr<Data> m_data;

            virtual BackendStatus init(const ProjectInfo &) override final;
            virtual bool addPhoto(Photo::Data &) override final;
            virtual bool update(const Photo::Data &) override final;
            virtual bool update(const TagNameInfo &) override final;

            virtual std::deque<TagNameInfo> listTags() override final;
            virtual std::deque<QVariant> listTagValues(const TagNameInfo&) override final;
            virtual std::deque<QVariant> listTagValues(const TagNameInfo &, const std::deque<IFilter::Ptr> &) override final;
            virtual std::deque<Photo::Id> getAllPhotos() override final;
            virtual Photo::Data getPhoto(const Photo::Id &) override final;
            virtual std::deque<Photo::Id> getPhotos(const std::deque<IFilter::Ptr> &) override final;
            virtual int getPhotosCount(const std::deque<IFilter::Ptr> &) override final;
            virtual std::deque<Photo::Id> dropPhotos(const std::deque<IFilter::Ptr> &) override final;

            BackendStatus checkStructure();
            Database::BackendStatus checkDBVersion();
    };

}

#endif // ASQLBACKEND_H

