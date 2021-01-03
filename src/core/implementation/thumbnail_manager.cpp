/*
 * Photo Broom - photos management tool.
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

#include "ithumbnails_cache.hpp"

using namespace std::placeholders;


ThumbnailManager::ThumbnailManager(ITaskExecutor* executor, IThumbnailsGenerator* gen, IThumbnailsCache* cache):
    m_tasks(executor, TasksQueue::Mode::Lifo),
    m_cache(cache),
    m_generator(gen)
{
}


void ThumbnailManager::fetch(const QString& path, int desired_height, const std::function<void(const QImage &)>& callback)
{
    internal_fetch(path, desired_height, callback);
}


void ThumbnailManager::fetch(const QString& path, int desired_height, const safe_callback<const QImage &>& callback)
{
    internal_fetch(path, desired_height, callback);
}


std::optional<QImage> ThumbnailManager::fetch(const QString& path, int height)
{
    std::optional img = m_cache->find(path, height);

    return img;
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


void ThumbnailManager::generate(const QString& path, int desired_height, const safe_callback<const QImage &>& callback)
{
    runOn(&m_tasks, [=, this]
    {
        if (callback.is_valid())        // callback may have become invalid, do not calculate anything to save CPU
            generate_task(path, desired_height, callback);
    });
}


void ThumbnailManager::generate(const QString& path, int desired_height, const std::function<void(const QImage &)>& callback)
{
    runOn(&m_tasks, [=, this]
    {
        generate_task(path, desired_height, callback);
    });
}

