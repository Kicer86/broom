/*
 * Class responsible for managing thumbnails
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

#include "thumbnail_manager.hpp"

#include "iimage_cache.hpp"


ThumbnailManager::ThumbnailManager(AThumbnailGenerator* gen):
    AThumbnailManager(gen),
    m_cache(nullptr)
{
}


void ThumbnailManager::setCache(IImageCache* cache)
{
    m_cache = cache;
}


QImage ThumbnailManager::find(const QString& path, int height)
{
    QImage result;

    if (m_cache)
    {
        const auto cached = m_cache->find(path, height);

        if (cached.has_value())
            result = *cached;
    }

    return result;
}


void ThumbnailManager::cache(const QString& path, int height, const QImage& img)
{
    if (m_cache)
        m_cache->store(path, height, img);
}

