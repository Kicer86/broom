/*
 * Photo Broom - photos management tool.
 * Copyright (C) 2016  Michał Walenciak <Kicer86@gmail.com>
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

#ifndef SELECTIONEXTRACTOR_HPP
#define SELECTIONEXTRACTOR_HPP

#include <vector>

#include <database/photo_data.hpp>

class QItemSelectionModel;

class APhotoInfoModel;


class SelectionExtractor final: public QObject
{
        Q_OBJECT

    public:
        SelectionExtractor();
        ~SelectionExtractor();

        SelectionExtractor(const SelectionExtractor &) = delete;
        SelectionExtractor& operator=(const SelectionExtractor &) = delete;

        void set(const QItemSelectionModel *);
        void set(APhotoInfoModel *);

        std::vector<Photo::Data> getSelection() const;

    private:
        const QItemSelectionModel* m_selectionModel;
        APhotoInfoModel* m_photosModel;

    signals:
        void selectionChanged();
};

#endif // SELECTIONEXTRACTOR_HPP
