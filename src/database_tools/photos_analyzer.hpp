/*
 * Photo analyzer and updater.
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

#ifndef PHOTOS_ANALYZER_HPP
#define PHOTOS_ANALYZER_HPP

#include <QObject>

#include <database/iphoto_info.hpp>

#include "database_tools_export.h"


namespace Database
{
    struct IDatabase;
}

class DATABASE_TOOLS_EXPORT PhotosAnalyzer: QObject
{
        Q_OBJECT

    public:
        struct Impl;
        
        PhotosAnalyzer();
        PhotosAnalyzer(const PhotosAnalyzer &) = delete;
        ~PhotosAnalyzer();

        PhotosAnalyzer& operator=(const PhotosAnalyzer &) = delete;

        void setDatabase(Database::IDatabase *);

    private:
        Impl* m_data;

    private slots:
        void photoAdded(const IPhotoInfo::Ptr &);

};

#endif // PHOTOS_ANALYZER_HPP
