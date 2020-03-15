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

#ifndef COREFACTORYQOBJECT_HPP
#define COREFACTORYQOBJECT_HPP

#include <QObject>

#include "icore_factory_accessor.hpp"
#include "core_export.h"

class CORE_EXPORT CoreFactoryQObject: public QObject
{
        Q_OBJECT

    public:
        CoreFactoryQObject(ICoreFactoryAccessor &);

        Q_INVOKABLE ICoreFactoryAccessor& get() const;

    private:
        ICoreFactoryAccessor& m_core;
};

#endif // COREFACTORYQOBJECT_HPP