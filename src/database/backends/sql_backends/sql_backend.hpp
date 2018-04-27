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
#include <vector>

#include "database/ibackend.hpp"
#include "sql_backend_base_export.h"
#include "table_definition.hpp"

class QSqlQuery;
class QSqlDatabase;

struct IPhotoInfoManager;

namespace Database
{

    class Entry;
    class InsertQueryData;
    struct IGenericSqlQueryGenerator;
    struct TableDefinition;

    class SQL_BACKEND_BASE_EXPORT ASqlBackend: public Database::IBackend
    {
        public:
            struct Data;

            ASqlBackend();
            ASqlBackend(const ASqlBackend& other) = delete;
            virtual ~ASqlBackend() ;

            ASqlBackend& operator=(const ASqlBackend& other) = delete;
            bool operator==(const ASqlBackend& other) = delete;

            void closeConnections() override;

            const QString& getConnectionName() const;

        protected:
            //will be called from init(). Prepare QSqlDatabase object here
            virtual BackendStatus prepareDB(const ProjectInfo& location) = 0;

            // called when db opened. Backend may perform some extra setup
            virtual bool dbOpened();

            //make sure table exists. Makes sure a table maching TableDefinition exists in database
            BackendStatus ensureTableExists(const TableDefinition &) const;

            //execute query. Function for inheriting classes
            virtual bool exec(const QString &, QSqlQuery *) const;

            virtual const IGenericSqlQueryGenerator* getGenericQueryGenerator() const = 0;

            virtual void set(ILoggerFactory *) override;

        private:
            std::unique_ptr<Data> m_data;

            // Database::IBackend:
            BackendStatus init(const ProjectInfo &) override final;
            bool addPhotos(std::vector<Photo::DataDelta> &) override final;
            Group::Id addGroup(const Photo::Id & ) override final;
            bool update(const Photo::DataDelta &) override final;

            std::vector<TagNameInfo> listTags() override final;
            std::vector<TagValue>    listTagValues(const TagNameInfo&) override final;
            std::vector<TagValue>    listTagValues(const TagNameInfo &, const std::vector<IFilter::Ptr> &) override final;

            std::vector<Photo::Id>   getAllPhotos() override final;
            std::vector<Photo::Id>   getPhotos(const std::vector<IFilter::Ptr> &) override final;
            std::vector<Photo::Id>   dropPhotos(const std::vector<IFilter::Ptr> &) override final;
            Photo::Data              getPhoto(const Photo::Id &) override final;
            int                      getPhotosCount(const std::vector<IFilter::Ptr> &) override final;
            QList<QVariant>          find(const QString &) override final;
            std::vector<PersonData>  listPeople() override final;
            std::vector<PersonLocation> listPeople (const Photo::Id &) override final;
            Person::Id               store(const PersonData &) override final;
            void                     store(const Photo::Id &,
                                           const Person::Id &,
                                           const QRect &) override;

            virtual void perform(const std::vector<IFilter::Ptr> &, const std::vector<IAction::Ptr> &) override final;
            //

            BackendStatus checkStructure();
            Database::BackendStatus checkDBVersion();

            bool createKey(const Database::TableDefinition::KeyDefinition &, const QString &, QSqlQuery &) const;
    };

}

#endif // ASQLBACKEND_H

