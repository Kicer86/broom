/*
 * Photo Broom - photos management tool.
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

#include "filter.hpp"

namespace Database
{

    GroupFilter::GroupFilter(const std::vector<Filter>& f)
        : filters(f)
    {
    }

    GroupFilter::GroupFilter(const std::initializer_list<Filter>& f)
        : filters(f)
    {
    }


    FilterPhotosWithTag::FilterPhotosWithTag(const TagTypes& type, const TagValue& value, ValueMode mode, bool include_empty):
        tagType(type),
        tagValue(value),
        valueMode(mode),
        includeEmpty(include_empty)
    {

    }


    FilterPhotosWithFlags::FilterPhotosWithFlags(): flags(), mode(Mode::And)
    {

    }


    FilterPhotosWithFlags::FilterPhotosWithFlags(const std::map<Photo::FlagsE, int>& f)
        : flags(f)
        , mode(Mode::And)
    {

    }


    FilterPhotosWithSha256::FilterPhotosWithSha256(): sha256("")
    {

    }


    FilterNotMatchingFilter::FilterNotMatchingFilter(const Filter& f): filter(new Filter(f))
    {

    }


    FilterPhotosWithId::FilterPhotosWithId(): filter()
    {

    }


    FilterPhotosMatchingExpression::FilterPhotosMatchingExpression(const SearchExpressionEvaluator::Expression& expr): expression(expr)
    {

    }


    FilterPhotosWithPath::FilterPhotosWithPath(const QString& p): path(p)
    {

    }


    FilterPhotosWithRole::FilterPhotosWithRole(Database::FilterPhotosWithRole::Role role): m_role(role)
    {

    }


    Database::FilterPhotosWithPerson::FilterPhotosWithPerson(const Person::Id& id):
        person_id(id)
    {
    }


    FilterPhotosWithGeneralFlag::FilterPhotosWithGeneralFlag(const QString& n, int v)
        : name(n)
        , value(v)
    {
    }
}
