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

#include "base_tags.hpp"

#include <QObject>


std::vector<BaseTagsList> BaseTags::getAll()
{
    const std::vector<BaseTagsList> tags =
    {
        BaseTagsList::Event,
        BaseTagsList::Place,
        BaseTagsList::Date,
        BaseTagsList::Time,
        BaseTagsList::People,
    };

    return tags;
}


QString BaseTags::getTr(BaseTagsList tag)
{
    QString result;

    switch (tag)
    {
        case BaseTagsList::Invalid:
            break;

        case BaseTagsList::Event:
            result = tr("Event");
            break;

        case BaseTagsList::Place:
            result = tr("Place");
            break;

        case BaseTagsList::Date:
            result = tr("Date");
            break;

        case BaseTagsList::Time:
            result = tr("Time");
            break;

        case BaseTagsList::People:
            result = tr("People");
            break;
    }

    return result;
}



QString BaseTags::getName(BaseTagsList tag)
{
    QString result;

    switch (tag)
    {
        case BaseTagsList::Invalid:
            break;

        case BaseTagsList::Event:
            result = "Event";
            break;

        case BaseTagsList::Place:
            result = "Place";
            break;

        case BaseTagsList::Date:
            result = "Date";
            break;

        case BaseTagsList::Time:
            result = "Time";
            break;

        case BaseTagsList::People:
            result = "People";
            break;
    }

    return result;
}


TagType BaseTags::getType(BaseTagsList tag)
{
    TagType result = TagType::Empty;

    switch (tag)
    {
        case BaseTagsList::Invalid:
            break;

        case BaseTagsList::Event:
            result = TagType::String;
            break;

        case BaseTagsList::Place:
            result = TagType::String;
            break;

        case BaseTagsList::Date:
            result = TagType::Date;
            break;

        case BaseTagsList::Time:
            result = TagType::Time;
            break;

        case BaseTagsList::People:
            result = TagType::List;
            break;
    }

    return result;
}

