/*
 * Utility for translating items relative positions to absolute ones.
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

#ifndef POSITIONSTRANSLATOR_HPP
#define POSITIONSTRANSLATOR_HPP

#include <QRect>

#include "data.hpp"

struct ModelIndexInfo;

class PositionsTranslator
{
    public:
        PositionsTranslator(const Data *);
        PositionsTranslator(const PositionsTranslator &) = delete;
        ~PositionsTranslator();

        [[deprecated]] QRect getAbsoluteRect(const Data::ModelIndexInfoSet::Model::const_level_iterator &) const;
        QRect getAbsoluteRect(const QModelIndex &) const;
        [[deprecated]] QRect getAbsoluteOverallRect(const Data::ModelIndexInfoSet::Model::const_iterator &) const;
        QRect getAbsoluteOverallRect(const QModelIndex &) const;
        [[deprecated]] QPoint getAbsolutePosition(const Data::ModelIndexInfoSet::Model::const_level_iterator &) const;
        QPoint getAbsolutePosition(const QModelIndex &) const;

        PositionsTranslator& operator=(const PositionsTranslator &) = delete;

    private:
        const Data* m_data;
};

#endif // POSITIONSTRANSLATOR_HPP
