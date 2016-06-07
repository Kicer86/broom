/*
 * Thumbnail Acquisitor
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


#include "thumbnail_acquisitor.hpp"

ThumbnailAcquisitor::ThumbnailAcquisitor():
    m_observers(),
    m_inProgress(),
    m_cacheAccessMutex(),
    m_generator(),
    m_cache()
{

}


ThumbnailAcquisitor::~ThumbnailAcquisitor()
{

}


void ThumbnailAcquisitor::set(ITaskExecutor* executor)
{
    m_generator.set(executor);
}


void ThumbnailAcquisitor::set(IPhotosManager* manager)
{
    m_generator.set(manager);
}


void ThumbnailAcquisitor::setInProgressThumbnail(const QImage& image)
{
    m_inProgress = image;
}


void ThumbnailAcquisitor::setObserver(const Observer& observer)
{
    m_observers.push_back(observer);
}


void ThumbnailAcquisitor::dismissPedingTasks()
{
    m_generator.dismissPedingTasks();
}


QImage ThumbnailAcquisitor::getThumbnail(const ThumbnailInfo& info) const
{
    QImage result;

    std::lock_guard<std::mutex> lock(m_cacheAccessMutex);

    auto image = m_cache.get(info);

    if (image)
        result = *image;
    else
    {
        // store temporary image in cache,
        // so new requests for it will not call generation again
        m_cache.add(info, m_inProgress);
        result = m_inProgress;

        auto callback =  std::bind(&ThumbnailAcquisitor::gotThumbnail, this, std::placeholders::_1, std::placeholders::_2);
        m_generator.generateThumbnail(info, callback);
    }

    return result;
}


void ThumbnailAcquisitor::gotThumbnail(const ThumbnailInfo& info, const QImage& image) const
{
    std::lock_guard<std::mutex> lock(m_cacheAccessMutex);

    m_cache.add(info, image);

    for(const Observer& obs: m_observers)
        obs(info, image);
}
