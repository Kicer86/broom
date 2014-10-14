/*
 * Utility for photos collecting.
 * Copyright (C) 2014  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "photos_collector.hpp"

#include <cassert>

#include <analyzer/photo_crawler_builder.hpp>

#include "components/staged_photos_data_model.hpp"


PhotosReceiver::PhotosReceiver(): m_model(nullptr)
{

}


void PhotosReceiver::setModel(StagedPhotosDataModel *model)
{
    m_model = model;
}


void PhotosReceiver::found(const QString &path)
{
    m_model->addPhoto(path);
}


///////////////////////////////////////////////////////////////////////////////


struct PhotosCollector::Data
{
    StagedPhotosDataModel* m_model;
    IPhotoCrawler* m_crawler;
    PhotosReceiver m_receiver;

    Data(): m_model(nullptr), m_crawler(nullptr), m_receiver()
    {
        m_crawler = PhotoCrawlerBuilder().build();
    }

    Data(const Data &) = delete;
    Data& operator=(const Data &) = delete;
};


PhotosCollector::PhotosCollector(): m_data(new Data)
{
    connect(&m_data->m_receiver, SIGNAL(finished()), this, SIGNAL(finished()));
}


PhotosCollector::~PhotosCollector()
{

}


void PhotosCollector::set(StagedPhotosDataModel* model)
{
    m_data->m_model = model;
    m_data->m_receiver.setModel(model);
}


void PhotosCollector::addDir(const QString& path)
{
    assert(m_data->m_model != nullptr);

    m_data->m_crawler->crawl(path, &m_data->m_receiver);
}
