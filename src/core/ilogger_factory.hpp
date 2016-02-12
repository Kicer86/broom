/*
 * Interface for LoggerFactory
 * Copyright (C) 2015  Michał Walenciak <MichalWalenciak@gmail.com>
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

#ifndef ILOGGERFACTORY_HPP
#define ILOGGERFACTORY_HPP

#include <memory>
#include <vector>

#include <QString>

struct ILogger;

struct ILoggerFactory
{
    virtual ~ILoggerFactory();

    virtual std::unique_ptr<ILogger> get(const QString& utility) = 0;
    virtual std::unique_ptr<ILogger> get(const std::vector<QString>& utility) = 0;
};

#endif // ILOGGERFACTORY_HPP
