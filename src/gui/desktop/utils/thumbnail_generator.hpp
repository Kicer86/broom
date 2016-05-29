/*
 * Low level thumbnails generator and catcher.
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

#ifndef THUMBNAILGENERATOR_HPP
#define THUMBNAILGENERATOR_HPP

#include <mutex>

#include <QCache>

#include "ithumbnail_generator.hpp"

struct IPhotosManager;
struct ITaskExecutor;

class ThumbnailGenerator: public IThumbnailGenerator
{
    public:
        ThumbnailGenerator();
        ThumbnailGenerator(const ThumbnailGenerator &) = delete;
        ~ThumbnailGenerator();

        ThumbnailGenerator& operator=(const ThumbnailGenerator &) = delete;

        void set(ITaskExecutor *);
        void set(IPhotosManager *);
        void generateThumbnail(const ThumbnailInfo &, const Callback &) const override;

    private:
        ITaskExecutor* m_executor;
        IPhotosManager* m_photosManager;
};


class ThumbnailCache: public IThumbnailCache
{
    public:
        ThumbnailCache();
        ThumbnailCache(const ThumbnailCache &) = delete;
        ~ThumbnailCache();

        ThumbnailCache& operator=(const ThumbnailCache &) = delete;

        void add(const ThumbnailInfo &, const QImage &) override;
        boost::optional<QImage> get(const ThumbnailInfo &) const override;

    private:
        mutable std::mutex m_cacheMutex;
        QCache<ThumbnailInfo, QImage> m_cache;
};

#endif // THUMBNAILGENERATOR_HPP