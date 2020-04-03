/*
 * Copyright (C) 2020  Michał Walenciak <Kicer86@gmail.com>
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
 */

#include "photos_model_component.hpp"

#include "models/flat_model.hpp"


PhotosModelComponent::PhotosModelComponent(QObject* p)
    : QObject(p)
    , m_model(new FlatModel(this))
{
}


void PhotosModelComponent::setDatabase(Database::IDatabase* db)
{
    m_db = db;
    m_model->setDatabase(db);
}


QAbstractItemModel* PhotosModelComponent::model() const
{
    return m_model;
}


const QDate& PhotosModelComponent::timeRangeFrom() const
{
    return m_timeRange.first;
}


const QDate& PhotosModelComponent::timeRangeTo() const
{
    return m_timeRange.second;
}


const QDate& PhotosModelComponent::timeViewFrom() const
{
    return m_timeView.first;
}


const QDate& PhotosModelComponent::timeViewTo() const
{
    return m_timeView.second;
}


void PhotosModelComponent::setTimeViewFrom(const QDate& viewFrom)
{
    m_timeView.first = viewFrom;
}


void PhotosModelComponent::setTimeViewTo(const QDate& viewTo)
{
    m_timeView.second = viewTo;
}


void PhotosModelComponent::setTimeRange(const QDate& from, const QDate& to)
{
    const QPair newTimeRange(from, to);

    if (newTimeRange != m_timeRange)
    {
        m_timeRange = QPair(from, to);
        m_timeView = m_timeRange;

        emit timeRangeFromChanged();
        emit timeRangeToChanged();

        //m_db->exec(std::bind(&FlatModel::fetchMatchingPhotos, this, _1));
    }
}
