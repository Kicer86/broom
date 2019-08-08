/*
 * Dialog for series detection
 * Copyright (C) 2019  Michał Walenciak <Kicer86@gmail.com>
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

#ifndef SERIESDETECTION_HPP
#define SERIESDETECTION_HPP

#include <QDialog>

#include <core/function_wrappers.hpp>
#include <database/database_tools/series_detector.hpp>

class QStandardItemModel;

namespace Database
{
    struct IDatabase;
    struct IBackend;
}

struct IExifReaderFactory;


class SeriesDetection: public QDialog
{
        Q_OBJECT

    public:
        SeriesDetection(Database::IDatabase *, IExifReaderFactory *);
        ~SeriesDetection();

    private:
        safe_callback_ctrl m_callback_mgr;
        QStandardItemModel* m_tabModel;
        IExifReaderFactory* m_exif;

        void fetch_series(Database::IBackend *);
        void load_series(const std::vector<SeriesDetector::Detection> &);
};

#endif // SERIESDETECTION_HPP