/*
 * Set of base tags
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

#ifndef BASETAGS_H
#define BASETAGS_H

#include <map>
#include <vector>

#include "tag.hpp"

enum class BaseTagsList
{
    Event,
    Place,
    Date,
    Time,
    People
};

class BaseTags
{
    static std::map<BaseTagsList, TagNameInfo> m_base_tags;
    static std::vector<TagNameInfo> collectTags();

    public:
        BaseTags() = delete;

        static TagNameInfo get(const BaseTagsList &);
        static const std::vector<TagNameInfo>& getAll();
};

#endif // BASETAGS_H
