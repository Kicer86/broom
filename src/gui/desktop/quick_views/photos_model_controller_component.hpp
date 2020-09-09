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

#ifndef PHOTOSMODELCOMPONENT_HPP
#define PHOTOSMODELCOMPONENT_HPP

#include <QDate>
#include <QObject>
#include <QTimer>

#include <database/idatabase.hpp>
#include <database/filter.hpp>
#include "models/flat_model.hpp"


class QAbstractItemModel;


class PhotosModelControllerComponent: public QObject
{
        Q_OBJECT

        Q_PROPERTY(QAbstractItemModel* photos READ model NOTIFY modelChanged)
        Q_PROPERTY(unsigned int datesCount READ datesCount NOTIFY datesCountChanged)
        Q_PROPERTY(unsigned int timeViewFrom READ timeViewFrom WRITE setTimeViewFrom)
        Q_PROPERTY(unsigned int timeViewTo READ timeViewTo WRITE setTimeViewTo)
        Q_PROPERTY(QString searchExpression READ searchExpression WRITE setSearchExpression)
        Q_PROPERTY(bool newPhotosOnly READ newPhotosOnly WRITE setNewPhotosOnly)

    public:
        PhotosModelControllerComponent(QObject * = nullptr);

        void setDatabase(Database::IDatabase *);

        APhotoInfoModel* model() const;

        unsigned int datesCount() const;
        unsigned int timeViewFrom() const;
        unsigned int timeViewTo() const;
        QString searchExpression() const;
        bool newPhotosOnly() const;

        void setTimeViewFrom(unsigned int);
        void setTimeViewTo(unsigned int);
        void setSearchExpression(const QString &);
        void setNewPhotosOnly(bool);

        Q_INVOKABLE QDate dateFor(unsigned int) const;
        Q_INVOKABLE void markNewAsReviewed();

    signals:
        void modelChanged() const;
        void datesCountChanged() const;

    private:
        QTimer m_searchLauncher;
        std::vector<QDate> m_dates;
        QPair<unsigned int, unsigned int> m_timeView;
        QString m_searchExpression;
        FlatModel* m_model;
        Database::IDatabase* m_db;
        bool m_newPhotosOnly;

        void updateModelFilters();
        void setAvailableDates(const std::vector<TagValue> &);
        void updateTimeRange();
        std::vector<Database::IFilter::Ptr> allFilters() const;

        void getTimeRangeForFilters(Database::IBackend &);
        void markPhotosAsReviewed(Database::IBackend &);
};

#endif // PHOTOSMODELCOMPONENT_HPP
