/*
 * class used for finding right place in tree for new photos
 * Copyright (C) 2014  Michał Walenciak <email>
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

#include "photos_matcher.hpp"

#include "idx_data_manager.hpp"


struct FiltersMatcher: Database::IFilterVisitor
{
    FiltersMatcher();
    FiltersMatcher(const FiltersMatcher &) = delete;
    virtual ~FiltersMatcher();

    FiltersMatcher& operator=(const FiltersMatcher &) = delete;

    bool doesMatch(const IPhotoInfo::Ptr &, const std::deque<Database::IFilter::Ptr> &);
    bool doesMatch(const IPhotoInfo::Ptr &, const Database::IFilter::Ptr &);

    private:
        bool m_doesMatch;
        IPhotoInfo* m_photo;

        virtual void visit(Database::FilterEmpty *) override;
        virtual void visit(Database::FilterDescription *) override;
        virtual void visit(Database::FilterFlags *) override;
};


FiltersMatcher::FiltersMatcher(): m_doesMatch(false), m_photo(nullptr)
{

}


FiltersMatcher::~FiltersMatcher()
{

}


bool FiltersMatcher::doesMatch(const IPhotoInfo::Ptr& photoInfo, const std::deque<Database::IFilter::Ptr>& filters)
{
    m_doesMatch = false;
    m_photo = photoInfo.get();

    for(const Database::IFilter::Ptr& filter: filters)
    {
        filter->visitMe(this);

        if (m_doesMatch == false)
            break;
    }

    m_photo = nullptr;
    return m_doesMatch;
}


bool FiltersMatcher::doesMatch(const IPhotoInfo::Ptr& photoInfo, const Database::IFilter::Ptr& filter)
{
    m_doesMatch = false;
    m_photo = photoInfo.get();

    filter->visitMe(this);

    m_photo = nullptr;
    return m_doesMatch;
}


void FiltersMatcher::visit(Database::FilterEmpty *)
{
    m_doesMatch = true;
}


void FiltersMatcher::visit(Database::FilterDescription* filter)
{
    bool result = false;

    const std::shared_ptr<ITagData> tags = m_photo->getTags();
    const ITagData::TagsList tagsList = tags->getTags();

    auto it = tagsList.find(filter->tagName);

    if (it != tagsList.end())
    {
        const std::set<TagValueInfo>& vals = it->second;

        result = vals.find(filter->tagValue) != vals.end();
    }

    m_doesMatch = result;
}


void FiltersMatcher::visit(Database::FilterFlags* filter)
{
    const bool status = m_photo->getFlags().stagingArea == filter->stagingArea;

    m_doesMatch = status;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

PhotosMatcher::PhotosMatcher(): m_idxDataManager(nullptr), m_dbDataModel(nullptr)
{

}


PhotosMatcher::~PhotosMatcher()
{

}


void PhotosMatcher::set(IdxDataManager* manager)
{
    m_idxDataManager = manager;
}


void PhotosMatcher::set(DBDataModel* model)
{
    m_dbDataModel = model;
}


bool PhotosMatcher::doesMatchModelFilters(const IPhotoInfo::Ptr& photoInfo) const
{
    std::deque<Database::IFilter::Ptr> filters = m_dbDataModel->getModelSpecificFilters();

    FiltersMatcher matcher;
    const bool result = matcher.doesMatch(photoInfo, filters);

    return result;
}


bool PhotosMatcher::doesMatchFilter(const IPhotoInfo::Ptr& photoInfo, const Database::IFilter::Ptr& filter)
{
    FiltersMatcher matcher;
    const bool result = matcher.doesMatch(photoInfo, filter);

    return result;
}


IdxData* PhotosMatcher::findParentFor(const IPhotoInfo::Ptr& photoInfo) const
{
    IdxData* root = m_idxDataManager->getRoot();

    return findParentFor(photoInfo, root);
}


IdxData* PhotosMatcher::findParentFor(const IPhotoInfo::Ptr& photoInfo, IdxData* current) const
{
    const size_t depth = m_idxDataManager->getHierarchy().levels.size();
    IdxData* result = nullptr;
    std::deque<IdxData *> toCheck = { current };
    FiltersMatcher matcher;

    while (result == false && toCheck.empty() == false)
    {
        IdxData* check = toCheck.front();
        toCheck.pop_front();

        assert(check->isNode());

        const Database::IFilter::Ptr& filter = check->m_filter;
        const bool matches = matcher.doesMatch(photoInfo, filter);

        if (matches)                         //does match - yeah
        {
            if (check->m_level < depth)      //go thru children. Better match may happen
                toCheck.insert(toCheck.end(), check->m_children.begin(), check->m_children.end());
            else
                result = check;              //save result
        }
    }

    return result;
}
