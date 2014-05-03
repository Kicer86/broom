/*
 * SQLDatabase iterator
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

#include "photo_iterator.hpp"

#include <QVariant>

#include <core/tag.hpp>
#include <database/idatabase.hpp>

#include "implementation/db_photo_info.hpp"

namespace Database
{

    struct PhotoIterator::Impl
    {
        InterfaceContainer<IQuery> m_query;
        PhotoInfo::Ptr m_photoInfo;
        PhotoIterator* m_iterator;

        Impl(const InterfaceContainer<IQuery>& query, PhotoIterator* iterator): m_query(query), m_photoInfo(), m_iterator(iterator)
        {
            m_query->gotoNext();
        }

        Impl(): m_query(), m_photoInfo(), m_iterator(nullptr)
        {
        }

        Impl(const Impl &) = default;

        void calculatePhotoInfo()
        {
            if (m_photoInfo.get() == nullptr)
            {
                //load photo data
                APhotoInfoInitData data;

                //const unsigned int id = m_query->getField(IQuery::Fields::Id).toInt();
                data.path = m_query->getField(IQuery::Fields::Path).toString().toStdString();
                data.hash = m_query->getField(IQuery::Fields::Hash).toString().toStdString();

                m_photoInfo = std::make_shared<PhotoInfo>(data);

                //load tags
                const TagData tagData = m_query->backend()->getTagsFor(*m_iterator);

                std::shared_ptr<ITagData> tags = m_photoInfo->getTags();
                tags->setTags(tags->getTags());
            }
        }

        void invalidate()
        {
            m_photoInfo.reset();
        }

        Impl& operator=(const Impl &) = default;
    };


    PhotoIterator::PhotoIterator(const InterfaceContainer<IQuery>& iquery): m_impl(new Impl(iquery, this))
    {

    }


    PhotoIterator::PhotoIterator(): m_impl(new Impl)
    {

    }


    PhotoIterator::~PhotoIterator()
    {

    }


    PhotoIterator::operator bool() const
    {
        const bool result = *m_impl->m_query != nullptr &&
                            m_impl->m_query->valid();
        return result;
    }


    bool PhotoIterator::operator!() const
    {
        const bool result = *m_impl->m_query != nullptr &&
                            m_impl->m_query->valid();
        return !result;
    }


    PhotoIterator& PhotoIterator::operator++()
    {
        if ( *this )
        {
            //get id of current photo
            const unsigned int id = m_impl->m_query->getField(IQuery::Fields::Id).toInt();
            unsigned int n_id = 0;
            const bool status = m_impl->m_query->gotoNext();

            if (status)
                n_id = m_impl->m_query->getField(IQuery::Fields::Id).toInt();

            assert(!status || id != n_id);     //just a check if next photo is next one ;)

            m_impl->invalidate();              //any cached data is not valid now
        }

        return *this;
    }


    PhotoIterator PhotoIterator::operator++(int)
    {
        PhotoIterator other = *this;

        ++(*this);
        return other;
    }


    PhotoInfo::Ptr PhotoIterator::operator->()
    {
        m_impl->calculatePhotoInfo();
        return m_impl->m_photoInfo;
    }


    PhotoInfo::Ptr PhotoIterator::operator*()
    {
        m_impl->calculatePhotoInfo();
        return m_impl->m_photoInfo;
    }


    const IQuery* PhotoIterator::query() const
    {
        return *(m_impl->m_query);
    }

}
