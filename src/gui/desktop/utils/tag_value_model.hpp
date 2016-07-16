/*
 * Model for particular TagNameInfo values
 * Copyright (C) 2016  Michał Walenciak <MichalWalenciak@gmail.com>
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

#ifndef TAGVALUEMODEL_HPP
#define TAGVALUEMODEL_HPP

#include <QAbstractListModel>

#include <core/tag.hpp>

struct ILoggerFactory;
struct ITagInfoCollector;

class TagValueModel: public QAbstractListModel
{
    public:
        TagValueModel(const TagNameInfo &);
        TagValueModel(const TagValueModel &) = delete;
        ~TagValueModel();
        TagValueModel& operator=(const TagValueModel &) = delete;

        void set(ITagInfoCollector *);
        void set(ILoggerFactory *);

        // QAbstractListModel:
        virtual int rowCount(const QModelIndex &) const override;
        virtual QVariant data(const QModelIndex &, int) const override;

    private:
        std::vector<TagValue> m_values;
        TagNameInfo m_tagInfo;
        ITagInfoCollector* m_tagInfoCollector;
        ILoggerFactory* m_loggerFactory;
};

#endif // TAGVALUEMODEL_HPP
