/*
 * Unix version of factory for debugger
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

#include "debugger_factory.hpp"

#include <iostream>

#include <QStandardPaths>

#include "gdb_wrapper.hpp"


DebuggerFactory::DebuggerFactory()
{

}


DebuggerFactory::~DebuggerFactory()
{

}


std::unique_ptr<IDebugger> DebuggerFactory::get()
{
    const QString gdb = QStandardPaths::findExecutable("gdb");
    std::unique_ptr<IDebugger> result;

    if (gdb.isEmpty())
        std::cerr << "Could not find gdb!" << std::endl;
    else
        result.reset(new GDBWrapper(gdb));

    return result;
}

