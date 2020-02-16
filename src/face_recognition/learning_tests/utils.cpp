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

#include "utils.hpp"

#include <cmath>


#ifndef FACES_DIR
#error FACES_DIR is not set
#endif

#define STRING(s) #s
#define PATH(path) STRING(path)


namespace utils
{

    QString photoSetPath()
    {
        return PATH(FACES_DIR);
    }

    QString photoPath(int n)
    {
        const QString path = photoSetPath() + QString("/face%1.jpg").arg(n);

        return path;
    }

    QImage downsize(const QImage& source, int factor)
    {
        const QSize size = source.size();
        const qreal scaleFactor = std::sqrt(factor);
        const QSize scaledSize = size / scaleFactor;

        return source.scaled(scaledSize);
    }

}
