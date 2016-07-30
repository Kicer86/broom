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

#include "sql_backend.hpp"

#include <iostream>
#include <set>
#include <thread>
#include <chrono>
#include <sstream>

#include <QBuffer>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QVariant>
#include <QPixmap>

#include <OpenLibrary/utils/optional.hpp>

#include <core/tag.hpp>
#include <core/task_executor.hpp>
#include <core/ilogger.hpp>
#include <core/ilogger_factory.hpp>
#include <core/variant_converter.hpp>
#include <database/action.hpp>
#include <database/filter.hpp>
#include <database/iphoto_info_cache.hpp>
#include <database/project_info.hpp>
#include <database/photo_info.hpp>

#include "isql_query_constructor.hpp"
#include "tables.hpp"
#include "query_structs.hpp"
#include "sql_action_query_generator.hpp"
#include "sql_filter_query_generator.hpp"
#include "sql_query_executor.hpp"
#include "database_migrator.hpp"


// usefull links
// about insert + update/ignore: http://stackoverflow.com/questions/15277373/sqlite-upsert-update-or-insert


namespace Database
{
    namespace
    {
        QByteArray toPrintable(const QImage& image)
        {
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);

            image.save(&buffer, "JPEG");

            return bytes.toBase64();
        }

        QImage fromPrintable(const QByteArray& data)
        {
            const QByteArray bytes = QByteArray::fromBase64(data);
            QImage image;

            const bool status = image.loadFromData(bytes, "JPEG");

            if (status == false)
            {
                //TODO: load thumbnail from file
            }

            return image;
        }


        struct Transaction
        {
            Transaction(QSqlDatabase& db): m_db(db)
            {

            }

            ~Transaction()
            {

            }

            Transaction(const Transaction &) = delete;
            Transaction& operator=(const Transaction &) = delete;

            BackendStatus begin()
            {
                const BackendStatus status = m_db.transaction()? StatusCodes::Ok: StatusCodes::TransactionFailed;

                return status;
            }

            BackendStatus commit()
            {
                const BackendStatus status = m_db.commit()? StatusCodes::Ok: StatusCodes::TransactionCommitFailed;

                return status;
            }

        private:
            QSqlDatabase& m_db;

        };


        std::deque<TagValue> flatten(const TagValue& tagValue)
        {
            const TagType type = tagValue.type();

            std::deque<TagValue> result;

            switch (type)
            {
                case TagType::Empty:
                    assert(!"empty tag value!");
                    break;

                case TagType::Date:
                case TagType::String:
                case TagType::Time:
                    result.push_back(tagValue);
                    break;

                case TagType::List:
                {
                    auto list = tagValue.getList();

                    for (const TagValue& subitem: list)
                    {
                        const std::deque<TagValue> local_result = flatten(subitem);

                        result.insert(result.end(), local_result.begin(), local_result.end());
                    }

                    break;
                }
            }

            return result;
        }


        std::deque<std::pair<TagNameInfo, TagValue>> flatten(const Tag::TagsList& tagsList)
        {
            std::deque<std::pair<TagNameInfo, TagValue>> result;

            for(const auto& tag: tagsList)
            {
                const TagValue& tagValue = tag.second;
                const std::deque<TagValue> flatList = flatten(tagValue);

                for(const TagValue& flat: flatList)
                {
                    const auto p = std::make_pair(tag.first, flat);
                    result.push_back(p);
                }
            }

            return result;
        }

    }


    /*****************************************************************************/


    struct ASqlBackend::Data
    {
            ASqlBackend* m_backend;
            QString m_connectionName;
            std::unique_ptr<ILogger> m_logger;
            SqlQueryExecutor m_executor;
            bool m_dbHasSizeFeature;
            bool m_dbOpen;

            Data(ASqlBackend* backend);
            ~Data();
            Data(const Data &) = delete;
            Data& operator=(const Data &) = delete;

            ol::Optional<unsigned int> store(const TagNameInfo& nameInfo) const;
            bool                       store(const TagValue& value, int photo_id, int name_id, int tag_id = -1) const;

            bool insert(Photo::Data &);
            bool update(const Photo::Data &);

            std::deque<TagNameInfo> listTags() const;
            std::deque<QVariant>    listTagValues(const TagNameInfo& tagName);
            std::deque<QVariant>    listTagValues(const TagNameInfo &, const std::deque<IFilter::Ptr> &);

            void                  perform(const std::deque<IFilter::Ptr> &, const std::deque<IAction::Ptr> &);

            std::deque<Photo::Id> getPhotos(const std::deque<IFilter::Ptr> &);
            std::deque<Photo::Id> dropPhotos(const std::deque<IFilter::Ptr> &);
            Photo::Data           getPhoto(const Photo::Id &);
            int                   getPhotosCount(const std::deque<IFilter::Ptr> &);

        private:
            ol::Optional<unsigned int> findTagByName(const QString& name) const;

            bool storeData(const Photo::Data &) const;
            bool storeGeometryFor(const Photo::Id &, const QSize &) const;
            bool storeSha256(int photo_id, const Photo::Sha256sum &) const;
            bool storeTags(int photo_id, const Tag::TagsList &) const;
            bool storeFlags(const Photo::Data &) const;

            Tag::TagsList        getTagsFor(const Photo::Id &);
            QSize                getGeometryFor(const Photo::Id &);
            ol::Optional<Photo::Sha256sum> getSha256For(const Photo::Id &);
            void    updateFlagsOn(Photo::Data &, const Photo::Id &);
            QString getPathFor(const Photo::Id &);
            std::deque<Photo::Id> fetch(QSqlQuery &);

            bool updateOrInsert(const UpdateQueryData &) const;
    };


    ASqlBackend::Data::Data(ASqlBackend* backend): m_backend(backend),
                                                   m_connectionName(""),
                                                   m_logger(nullptr),
                                                   m_executor(),
                                                   m_dbHasSizeFeature(false),
                                                   m_dbOpen(false)
    {

    }


    ASqlBackend::Data::~Data()
    {

    }


    ol::Optional<unsigned int> ASqlBackend::Data::store(const TagNameInfo& nameInfo) const
    {
        const QString& name = nameInfo.getName();
        const int type = static_cast<int>( nameInfo.getType() );
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        //check if tag exists
        ol::Optional<unsigned int> tagId = findTagByName(name);

        if (! tagId)  //tag not yet in database
        {
            const QString queryStr = QString("INSERT INTO %1 (id, name, type) VALUES (NULL, '%2', '%3');")
                                     .arg(TAB_TAG_NAMES)
                                     .arg(name)
                                     .arg(type);

            const bool status = m_executor.exec(queryStr, &query);

            if (status)
                tagId = query.lastInsertId().toUInt();
        }

        return tagId;
    }


    bool ASqlBackend::Data::store(const TagValue& tagValue, int photo_id, int name_id, int tag_id) const
    {
        //store tag values
        bool status = true;
        const TagType type = tagValue.type();

        switch (type)
        {
            case TagType::Empty:
                assert(!"Empty tag value!");
                break;

            case TagType::List:
                assert(!"TagValue should be flat");
                break;

            case TagType::Date:
            case TagType::String:
            case TagType::Time:
            {
                QSqlDatabase db = QSqlDatabase::database(m_connectionName);
                QSqlQuery query(db);

                const QString value = tagValue.formattedValue();

                InsertQueryData queryData(TAB_TAGS);
                queryData.setColumns("id", "value", "photo_id", "name_id");

                if (tag_id == -1)
                    queryData.setValues(InsertQueryData::Value::Null,
                                        value,
                                        photo_id,
                                        name_id);
                else
                    queryData.setValues(tag_id,
                                        value,
                                        photo_id,
                                        name_id);

                auto query_str = m_backend->getGenericQueryGenerator()->insertOrUpdate(queryData);

                status = m_executor.exec(query_str, &query);

                break;
            }
        }

        return status;
    }


    std::deque<TagNameInfo> ASqlBackend::Data::listTags() const
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        const QString query_str("SELECT name, type FROM " TAB_TAG_NAMES ";");

        const bool status = m_executor.exec(query_str, &query);
        std::deque<TagNameInfo> result;

        while (status && query.next())
        {
            const QString name = query.value(0).toString();
            const int typeRaw = query.value(1).toInt();
            const TagType type = static_cast<TagType>(typeRaw);
            const TagNameInfo tagName(name, type);

            result.push_back(tagName);
        }

        return result;
    }


    std::deque<QVariant> ASqlBackend::Data::listTagValues(const TagNameInfo& tagName)
    {
        const ol::Optional<unsigned int> tagId = findTagByName(tagName);

        std::deque<QVariant> result;

        if (tagId)
        {
            VariantConverter convert;
            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            QSqlQuery query(db);
            const QString query_str = QString("SELECT value FROM " TAB_TAGS " WHERE name_id=\"%1\";")
                                      .arg(*tagId);

            const bool status = m_executor.exec(query_str, &query);

            while (status && query.next())
            {
                const QString raw_value = query.value(0).toString();
                const QVariant value = convert(tagName.getType(), raw_value);

                result.push_back(value);
            }
        }

        return result;
    }


    std::deque<QVariant> ASqlBackend::Data::listTagValues(const TagNameInfo& tagName, const std::deque<IFilter::Ptr>& filter)
    {
        const QString filterQuery = SqlFilterQueryGenerator().generate(filter);

        //from filtered photos, get info about tags used there
        QString queryStr = "SELECT DISTINCT %2.value FROM ( %1 ) AS distinct_select JOIN (%2, %3) ON (photos_id=%2.photo_id AND %3.id=%2.name_id) WHERE name='%4'";

        queryStr = queryStr.arg(filterQuery);
        queryStr = queryStr.arg(TAB_TAGS);
        queryStr = queryStr.arg(TAB_TAG_NAMES);
        queryStr = queryStr.arg(tagName.getName());

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        std::deque<QVariant> result;
        const bool status = m_executor.exec(queryStr, &query);

        if (status)
        {
            VariantConverter convert;

            while (status && query.next())
            {
                const QString raw_value = query.value(0).toString();
                const QVariant value = convert( tagName.getType(), raw_value);

                result.push_back(value);
            }
        }

        return result;
    }


    void ASqlBackend::Data::perform(const std::deque<IFilter::Ptr>& filter, const std::deque<IAction::Ptr>& actions)
    {
        for(auto action: actions)
        {
            const QString queryStr = SqlFilterQueryGenerator().generate(filter);
            const QString actionStr = SqlActionQueryGenerator().generate(action);
            const QString finalQuery = actionStr.arg(queryStr);

            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            QSqlQuery query(db);

            m_executor.exec(finalQuery, &query);
        }
    }


    std::deque<Photo::Id> ASqlBackend::Data::getPhotos(const std::deque<IFilter::Ptr>& filter)
    {
        const QString queryStr = SqlFilterQueryGenerator().generate(filter);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        m_executor.exec(queryStr, &query);
        auto result = fetch(query);

        return result;
    }


    int ASqlBackend::Data::getPhotosCount(const std::deque<IFilter::Ptr>& filter)
    {
        const QString queryStr = SqlFilterQueryGenerator().generate(filter);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        m_executor.exec(queryStr, &query);

        int result = 0;

        if (m_dbHasSizeFeature)
            result = query.size();
        else
            result = query.next()? 1: 0;

        return result;
    }


    std::deque<Photo::Id>  ASqlBackend::Data::dropPhotos(const std::deque<IFilter::Ptr>& filter)
    {
        const QString filterQuery = SqlFilterQueryGenerator().generate(filter);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        //collect ids of photos to be dropped
        std::deque<Photo::Id> ids;
        bool status = m_executor.exec(filterQuery, &query);

        if (status)
        {
            while(query.next())
            {
                Photo::Id id( query.value(0).toUInt() );

                ids.push_back(id);
            }
        }

        //from filtered photos, get info about tags used there
        std::vector<QString> queries =
        {
            QString("CREATE TEMPORARY TABLE drop_indices AS %1").arg(filterQuery),
            QString("DELETE FROM " TAB_FLAGS       " WHERE photo_id IN (SELECT * FROM drop_indices)"),
            QString("DELETE FROM " TAB_SHA256SUMS  " WHERE photo_id IN (SELECT * FROM drop_indices)"),
            QString("DELETE FROM " TAB_TAGS        " WHERE photo_id IN (SELECT * FROM drop_indices)"),
            QString("DELETE FROM " TAB_THUMBS      " WHERE photo_id IN (SELECT * FROM drop_indices)"),
            QString("DELETE FROM " TAB_PHOTOS      " WHERE id IN (SELECT * FROM drop_indices)"),
            QString("DROP TABLE drop_indices")
        };

        status = db.transaction();

        if (status)
            status = m_executor.exec(queries, &query);

        if (status)
            status = db.commit();
        else
            db.rollback();

        return ids;
    }


    ol::Optional<unsigned int> ASqlBackend::Data::findTagByName(const QString& name) const
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        const QString find_tag_query = QString("SELECT id FROM " TAB_TAG_NAMES " WHERE name =\"%1\";").arg(name);
        const bool status = m_executor.exec(find_tag_query, &query);

        ol::Optional<unsigned int> result;

        if (status && query.next())
            result = query.value(0).toInt();

        return result;
    }


    bool ASqlBackend::Data::storeData(const Photo::Data& data) const
    {
        assert(data.id);

        //store used tags
        Tag::TagsList tags = data.tags;

        bool status = storeTags(data.id, tags);

        if (status && data.getFlag(Photo::FlagsE::GeometryLoaded) > 0)
            status = storeGeometryFor(data.id, data.geometry);

        if (status && data.getFlag(Photo::FlagsE::Sha256Loaded) > 0)
            status = storeSha256(data.id, data.sha256Sum);

        if (status)
            status = storeFlags(data);

        return status;
    }


    bool ASqlBackend::Data::storeGeometryFor(const Photo::Id& photo_id, const QSize& geometry) const
    {
        InsertQueryData data(TAB_GEOMETRY);

        data.setColumns("photo_id", "width", "height");
        data.setValues(QString::number(photo_id), QString::number(geometry.width()), QString::number(geometry.height()) );

        const std::vector<QString> queryStrs = m_backend->getGenericQueryGenerator()->insertOrUpdate(data);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        bool status = m_executor.exec(queryStrs, &query);

        return status;
    }


    bool ASqlBackend::Data::storeSha256(int photo_id, const Photo::Sha256sum& sha256) const
    {
        InsertQueryData data(TAB_SHA256SUMS);

        data.setColumns("photo_id", "sha256");
        data.setValues(QString::number(photo_id), sha256.constData());

        const std::vector<QString> queryStrs = m_backend->getGenericQueryGenerator()->insertOrUpdate(data);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        bool status = m_executor.exec(queryStrs, &query);

        return status;
    }


    bool ASqlBackend::Data::storeTags(int photo_id, const Tag::TagsList& tagsList) const
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        bool status = true;

        // gather ids for current set of tag for photo_id
        const QString tagIdsQuery = QString("SELECT id FROM %1 WHERE photo_id=\"%2\"")
                                    .arg(TAB_TAGS)
                                    .arg(photo_id);

        status = m_executor.exec(tagIdsQuery, &query);

        if (status)
        {
            // read tag ids from query
            std::deque<int> currentIds;

            while (query.next())
            {
                const QVariant idRaw = query.value(0);
                const int id = idRaw.toInt();

                currentIds.push_back(id);
            }

            // convert map with possible lists into flat list of pairs
            const std::deque<std::pair<TagNameInfo, TagValue>> tagsFlatList = flatten(tagsList);

            // difference between current set in db and new set of tags
            const int currentIdsSize = static_cast<int>( currentIds.size() );
            const int tagsListSize   = static_cast<int>( tagsFlatList.size() );
            const int diff = currentIdsSize - tagsListSize;

            // more tags in db?, delete surplus
            if (diff > 0)
            {
                QStringList idsToDelete;
                for(auto it = currentIds.end() - diff; it != currentIds.end(); ++it)
                    idsToDelete.append( QString::number(*it) );

                const QString idsToDeleteStr = idsToDelete.join(", ");

                const QString deleteQuery = QString("DELETE FROM %1 WHERE id IN (%2)")
                                                .arg(TAB_TAGS)
                                                .arg(idsToDeleteStr);

                status = m_executor.exec(deleteQuery, &query);
            }

            // override existing tags and then insert (if nothing left to override)
            std::size_t counter = 0;

            for (auto it = tagsFlatList.begin(); status && it != tagsFlatList.end(); ++it, counter++)
            {
                //store tag name
                const ol::Optional<unsigned int> name_id = store(it->first);

                if (name_id)
                {
                    const TagValue& value = it->second;
                    const int nameid = *name_id;
                    const int tag_id = counter < currentIds.size()? currentIds[counter]: -1;  // try to override ids of tags already stored

                    status = store(value, photo_id, nameid, tag_id);
                }
                else
                    status = false;
            }

        }

        return status;
    }


    bool ASqlBackend::Data::storeFlags(const Photo::Data& photoData) const
    {
        UpdateQueryData queryInfo(TAB_FLAGS);
        queryInfo.setCondition("photo_id", QString::number(photoData.id));
        queryInfo.setColumns("photo_id", "staging_area", "tags_loaded", "sha256_loaded", "thumbnail_loaded", FLAG_GEOM_LOADED);
        queryInfo.setValues(QString::number(photoData.id),
                             photoData.getFlag(Photo::FlagsE::StagingArea),
                             photoData.getFlag(Photo::FlagsE::ExifLoaded),
                             photoData.getFlag(Photo::FlagsE::Sha256Loaded),
                             photoData.getFlag(Photo::FlagsE::ThumbnailLoaded),
                             photoData.getFlag(Photo::FlagsE::GeometryLoaded));

        auto queries = m_backend->getGenericQueryGenerator()->update(queryInfo);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        bool status = m_executor.exec(queries, &query);

        if (status)
        {
            const int affected_rows = query.numRowsAffected();

            if (affected_rows == 0)
            {
                queries = m_backend->getGenericQueryGenerator()->insert(queryInfo);
                status = m_executor.exec(queries, &query);
            }
        }

        return status;
    }


    bool ASqlBackend::Data::insert(Photo::Data& data)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        Transaction transaction(db);
        bool status = transaction.begin();

        //store path and date
        Photo::Id id = data.id;
        assert(id.valid() == false);

        InsertQueryData insertData(TAB_PHOTOS);

        insertData.setColumns("path", "store_date");
        insertData.setValues(data.path, InsertQueryData::Value::CurrentTime);
        insertData.setColumns("id");
        insertData.setValues(InsertQueryData::Value::Null);

        const std::vector<QString> queryStrs = m_backend->getGenericQueryGenerator()->insert(insertData);

        if (status)
            status = m_executor.exec(queryStrs, &query);

        //update id
        if (status)                                    //Get Id from database after insert
        {
            QVariant photo_id  = query.lastInsertId(); //TODO: WARNING: may not work (http://qt-project.org/doc/qt-5.1/qtsql/qsqlquery.html#lastInsertId)
            status = photo_id.isValid();

            if (status)
                id = Photo::Id(photo_id.toInt());
        }

        //make sure id is set
        if (status)
            status = id.valid();

        if (status)
        {
            assert(data.id.valid() == false || data.id == id);
            data.id = id;
        }

        if (status)
            status = storeData(data);

        if (status)
            status = transaction.commit();

        return status;
    }


    bool ASqlBackend::Data::update(const Photo::Data& data)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        Transaction transaction(db);
        bool status = transaction.begin();

        if (status)
            status = storeData(data);

        if (status)
            status = transaction.commit();

        return status;
    }


    Photo::Data ASqlBackend::Data::getPhoto(const Photo::Id& id)
    {
        Photo::Data photoData;
        photoData.path = getPathFor(id);
        photoData.id   = id;
        photoData.tags = getTagsFor(id);

        //load geometry
        const QSize geometry = getGeometryFor(id);
        if (geometry.isValid())
            photoData.geometry = geometry;

        //load sha256
        const ol::Optional<Photo::Sha256sum> sha256 = getSha256For(id);
        if (sha256)
            photoData.sha256Sum = *sha256;

        //load flags
        updateFlagsOn(photoData, id);

        return photoData;
    }


    Tag::TagsList ASqlBackend::Data::getTagsFor(const Photo::Id& photoId)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        const QString queryStr = QString("SELECT "
                                         "%1.id, %2.name, %1.value, %2.type "
                                         "FROM "
                                         "%1 "
                                         "JOIN "
                                         "%2 "
                                         "ON %2.id = %1.name_id "
                                         "WHERE %1.photo_id = '%3';")
                                 .arg(TAB_TAGS)
                                 .arg(TAB_TAG_NAMES)
                                 .arg(photoId.value());

        const bool status = m_executor.exec(queryStr, &query);
        Tag::TagsList tagData;

        while(status && query.next())
        {
            const QString name  = query.value(1).toString();
            const QVariant value = query.value(2);
            const int tagTypeRaw = query.value(3).toInt();
            const TagType tagType = static_cast<TagType>(tagTypeRaw);
            const TagNameInfo tagName(name, tagType);

            switch(tagType)
            {
                case TagType::Date:
                    tagData[tagName] = TagValue(value.toDate());
                    break;

                case TagType::List:
                {
                    // insert() will add empty List if there is no entry for given key.
                    // otherwise will do nothing.
                    auto insert_it = tagData.insert( std::make_pair(tagName, TagValueTraits<TagType::List>::StorageType()) );
                    auto it = insert_it.first;   // get regular map iterator
                    TagValue& tagValue = it->second;
                    TagValueTraits<TagType::List>::StorageType& values = tagValue.getList();

                    values.push_back( TagValue(value.toString()) );
                    break;
                }

                case TagType::Time:
                    tagData[tagName] = TagValue(value.toTime());
                    break;

                case TagType::String:
                    tagData[tagName] = TagValue(value.toString());
                    break;

                case TagType::Empty:
                    assert(!"invalid tag type");
                    break;
            }
        }

        return tagData;
    }


    QSize ASqlBackend::Data::getGeometryFor(const Photo::Id& id)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);

        QSize geoemtry;
        QSqlQuery query(db);

        const QString queryStr = QString("SELECT width,height FROM %1 WHERE %1.photo_id = '%2'")
                                 .arg(TAB_GEOMETRY)
                                 .arg(id.value());

        const bool status = m_executor.exec(queryStr, &query);

        if (status && query.next())
        {
            const QVariant widthRaw = query.value(0);
            const QVariant heightRaw = query.value(1);

            const int width = widthRaw.toInt();
            const int height = heightRaw.toInt();

            geoemtry = QSize(width, height);
        }

        return geoemtry;
    }


    ol::Optional<Photo::Sha256sum> ASqlBackend::Data::getSha256For(const Photo::Id& id)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        QString queryStr = QString("SELECT sha256 FROM %1 WHERE %1.photo_id = '%2'");

        queryStr = queryStr.arg(TAB_SHA256SUMS);
        queryStr = queryStr.arg(id.value());

        const bool status = m_executor.exec(queryStr, &query);

        ol::Optional<Photo::Sha256sum> result;
        if(status && query.next())
        {
            const QVariant variant = query.value(0);

            result->append(variant.toString());
        }

        return result;
    }


    void ASqlBackend::Data::updateFlagsOn(Photo::Data& photoData, const Photo::Id& id)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        QString queryStr = QString("SELECT staging_area, tags_loaded, sha256_loaded, thumbnail_loaded, geometry_loaded FROM %1 WHERE %1.photo_id = '%2'");

        queryStr = queryStr.arg(TAB_FLAGS);
        queryStr = queryStr.arg(id.value());

        const bool status = m_executor.exec(queryStr, &query);

        if (status && query.next())
        {
            QVariant variant = query.value(0);
            photoData.flags[Photo::FlagsE::StagingArea] = variant.toInt();

            variant = query.value(1);
            photoData.flags[Photo::FlagsE::ExifLoaded] = variant.toInt();

            variant = query.value(2);
            photoData.flags[Photo::FlagsE::Sha256Loaded] = variant.toInt();

            variant = query.value(3);
            photoData.flags[Photo::FlagsE::ThumbnailLoaded] = variant.toInt();

            variant = query.value(4);
            photoData.flags[Photo::FlagsE::GeometryLoaded] = variant.toInt();
        }
    }


    QString ASqlBackend::Data::getPathFor(const Photo::Id& id)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        QString queryStr = QString("SELECT path FROM %1 WHERE %1.id = '%2'");

        queryStr = queryStr.arg(TAB_PHOTOS);
        queryStr = queryStr.arg(id.value());

        const bool status = m_executor.exec(queryStr, &query);

        QString result;
        if(status && query.next())
        {
            const QVariant path = query.value(0);

            result = path.toString();
        }

        return result;
    }


    std::deque<Photo::Id> ASqlBackend::Data::fetch(QSqlQuery& query)
    {
        std::deque<Photo::Id> collection;

        while (query.next())
        {
            const Photo::Id id(query.value("photos_id").toInt());

            collection.push_back(id);
        }

        return collection;
    }


    bool ASqlBackend::Data::updateOrInsert(const UpdateQueryData& queryInfo) const
    {
        auto queries = m_backend->getGenericQueryGenerator()->update(queryInfo);

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);

        bool status = m_executor.exec(queries, &query);

        if (status)
        {
            const int affected_rows = query.numRowsAffected();

            if (affected_rows == 0)
            {
                queries = m_backend->getGenericQueryGenerator()->insert(queryInfo);
                status = m_executor.exec(queries, &query);
            }
        }

        return status;
    }


    ///////////////////////////////////////////////////////////////////////


    ASqlBackend::ASqlBackend(): m_data(new Data(this))
    {

    }


    ASqlBackend::~ASqlBackend()
    {
        assert(m_data->m_dbOpen == false);
    }


    void ASqlBackend::closeConnections()
    {
        {
            QSqlDatabase db = QSqlDatabase::database(m_data->m_connectionName);

            if (db.isValid() && db.isOpen())
            {
                m_data->m_logger->log(ILogger::Severity::Info, "ASqlBackend: closing database connections.");
                db.close();
                m_data->m_dbOpen = false;
            }
        }

        QSqlDatabase::removeDatabase(m_data->m_connectionName);
    }


    const QString& ASqlBackend::getConnectionName() const
    {
        return m_data->m_connectionName;
    }


    bool ASqlBackend::dbOpened()
    {
        return true;
    }


    BackendStatus ASqlBackend::assureTableExists(const TableDefinition& definition) const
    {
        QSqlDatabase db = QSqlDatabase::database(m_data->m_connectionName);

        QSqlQuery query(db);
        const QString showQuery = getGenericQueryGenerator()->prepareFindTableQuery(definition.name);

        BackendStatus status = m_data->m_executor.exec(showQuery, &query);

        //create table 'name' if doesn't exist
        bool empty = query.next() == false;

        if (status && empty)
        {
            QString columnsDesc;
            const int size = definition.columns.size();

            for(int i = 0; i < size; i++)
            {
                const QStringList types =
                {
                    getGenericQueryGenerator()->getTypeFor(definition.columns[i].purpose),
                    definition.columns[i].type_definition
                };

                const QString type = types.join(" ").simplified();

                const bool notlast = i + 1 < size;
                columnsDesc += definition.columns[i].name + " ";
                columnsDesc += type;
                columnsDesc += notlast? ", ": "";
            }

            status = m_data->m_executor.exec( getGenericQueryGenerator()->prepareCreationQuery(definition.name, columnsDesc), &query );

            if (status && definition.keys.empty() == false)
            {
                const int keys = definition.keys.size();

                for(int i = 0; status && i < keys; i++)
                    createKey(definition.keys[i], definition.name, query);
            }
        }

        return status;
    }


    bool ASqlBackend::exec(const QString& query, QSqlQuery* status) const
    {
        return m_data->m_executor.exec(query, status);
    }


    BackendStatus ASqlBackend::init(const ProjectInfo& prjInfo)
    {
        //store thread id for further validation
        m_data->m_executor.set( std::this_thread::get_id() );
        m_data->m_connectionName = prjInfo.databaseLocation;

        BackendStatus status = prepareDB(prjInfo);
        QSqlDatabase db = QSqlDatabase::database(m_data->m_connectionName);

        m_data->m_dbHasSizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);

        if (status)
        {
            m_data->m_dbOpen = db.open();
            status = m_data->m_dbOpen? StatusCodes::Ok: StatusCodes::OpenFailed;
        }

        if (status)
            status = dbOpened()? StatusCodes::Ok: StatusCodes::OpenFailed;

        if (status)
            status = checkStructure();
        else
            m_data->m_logger->error("Error opening database: " + db.lastError().text());

        return status;
    }


    bool ASqlBackend::addPhoto(Photo::Data& data)
    {
        const bool status = m_data->insert(data);

        return status;
    }


    bool ASqlBackend::update(const Photo::Data& data)
    {
        assert(data.id.valid());

        bool status = false;

        if (m_data)
            status = m_data->update(data);
        else
            m_data->m_logger->error("Database object does not exist.");

        return status;
    }


    bool ASqlBackend::update(const TagNameInfo& tagInfo)
    {
        assert(tagInfo.getType() != TagType::Empty);

        bool status = false;

        if (m_data)
            status = m_data->store(tagInfo);
        else
            m_data->m_logger->error("Database object does not exist.");

        return status;
    }


    std::deque<TagNameInfo> ASqlBackend::listTags()
    {
        std::deque<TagNameInfo> result;

        if (m_data)
            result = m_data->listTags();
        else
            m_data->m_logger->error("Database object does not exist.");

        return result;
    }


    std::deque<QVariant> ASqlBackend::listTagValues(const TagNameInfo& tagName)
    {
        std::deque<QVariant> result;

        if (m_data)
            result = m_data->listTagValues(tagName);
        else
            m_data->m_logger->error("Database object does not exist.");

        return result;
    }


    std::deque<QVariant> ASqlBackend::listTagValues(const TagNameInfo& tagName, const std::deque<IFilter::Ptr>& filter)
    {
        const std::deque<QVariant> result = m_data->listTagValues(tagName, filter);

        return result;
    }


    std::deque<Photo::Id> ASqlBackend::getAllPhotos()
    {
        std::deque<IFilter::Ptr> emptyFilter;
        return m_data->getPhotos(emptyFilter);  //like getPhotos but without any filters
    }


    Photo::Data ASqlBackend::getPhoto(const Photo::Id& id)
    {
        auto result = m_data->getPhoto(id);

        return result;
    }


    std::deque<Photo::Id> ASqlBackend::getPhotos(const std::deque<IFilter::Ptr>& filter)
    {
        return m_data->getPhotos(filter);
    }


    int ASqlBackend::getPhotosCount(const std::deque<IFilter::Ptr>& filter)
    {
        return m_data->getPhotosCount(filter);
    }


    void ASqlBackend::perform(const std::deque<IFilter::Ptr>& filter, const std::deque<IAction::Ptr>& action)
    {
        return m_data->perform(filter, action);
    }


    std::deque<Photo::Id> ASqlBackend::dropPhotos(const std::deque<IFilter::Ptr>& filter)
    {
        return m_data->dropPhotos(filter);
    }


    void ASqlBackend::set(ILoggerFactory* logger_factory)
    {
        m_data->m_logger = logger_factory->get({"Database" ,"ASqlBackend"});
        m_data->m_executor.set(m_data->m_logger.get());
    }


    BackendStatus ASqlBackend::checkStructure()
    {
        QSqlDatabase db = QSqlDatabase::database(m_data->m_connectionName);
        Transaction transaction(db);

        BackendStatus status = transaction.begin();

        //check tables existance
        if (status)
            for (const auto& table: tables)
            {
                status = assureTableExists(table.second);

                if (!status)
                    break;
            }

        QSqlQuery query(db);

        // table 'version' cannot be empty
        if (status)
            status = m_data->m_executor.exec("SELECT COUNT(*) FROM " TAB_VER ";", &query);

        if (status)
            status = query.next()? StatusCodes::Ok: StatusCodes::QueryFailed;

        const QVariant rows = status? query.value(0): QVariant(0);

        //insert first entry
        if (status)
        {
            if (rows == 0)
                status = m_data->m_executor.exec(QString("INSERT INTO " TAB_VER "(version) VALUES(%1);")
                                      .arg(db_version), &query);
            else
                status = checkDBVersion();
        }

        status &= transaction.commit();

        return status;
    }

}


Database::BackendStatus Database::ASqlBackend::checkDBVersion()
{
    QSqlDatabase db = QSqlDatabase::database(m_data->m_connectionName);

    QSqlQuery query(db);

    BackendStatus status = m_data->m_executor.exec("SELECT version FROM " TAB_VER ";", &query);

    if (status)
        status = query.next()? StatusCodes::Ok: StatusCodes::QueryFailed;

    if (status)
    {
        const int v = query.value(0).toInt();

        // More than we expect? Quit with error
        if (v > 1)
            status = StatusCodes::BadVersion;
    }

    // database update
    if (status)
    {
        status = m_data->m_executor.exec("SELECT version FROM " TAB_VER ";", &query);

        if (status)
            status = query.next()? StatusCodes::Ok: StatusCodes::QueryFailed;

        int v = 0;

        if (status)
            v = query.value(0).toInt();

        (void) v;
    }

    return status;
}


bool Database::ASqlBackend::createKey(const Database::TableDefinition::KeyDefinition& key, const QString& tableName, QSqlQuery& query) const
{
    QString indexDesc;

    indexDesc += "CREATE " + key.type;
    indexDesc += " " + key.name + "_idx";
    indexDesc += " ON " + tableName;
    indexDesc += " " + key.def + ";";

    const bool status = m_data->m_executor.exec(indexDesc, &query);

    return status;
}
