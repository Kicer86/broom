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

#ifndef PEOPLEMANIPULATOR_HPP
#define PEOPLEMANIPULATOR_HPP

#include <core/function_wrappers.hpp>
#include <database/photo_types.hpp>
#include <database/idatabase.hpp>

struct ICoreFactoryAccessor;

class PeopleManipulator: public QObject
{
    public:
        PeopleManipulator(const Photo::Id &, Database::IDatabase &, ICoreFactoryAccessor &);

    private:
        struct FaceInfo
        {
            QRect rect;

            FaceInfo(const QRect& r): rect(r) {}
        };

        safe_callback_ctrl m_callback_ctrl;
        std::vector<FaceInfo> m_faces;
        Photo::Id m_pid;
        ICoreFactoryAccessor& m_core;
        Database::IDatabase& m_db;

        void runOnThread(void (PeopleManipulator::*)());

        void findFaces();
        void findFaces_thrd();
        void findFaces_result(const QVector<QRect> &);

        void recognizeFaces();
        void recognizeFaces_thrd();
        void recognizeFaces_result();
};

#endif // PEOPLEMANIPULATOR_HPP
