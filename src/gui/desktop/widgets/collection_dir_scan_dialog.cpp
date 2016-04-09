/*
 * Dialog for collection scanning
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


#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "collection_dir_scan_dialog.hpp"


CollectionDirScanDialog::CollectionDirScanDialog(const QString& collectionLocation, Database::IDatabase* db, QWidget* p):
    QDialog(p),
    m_collector(),
    m_photosFound(),
    m_dbPhotos(),
    m_collectionLocation(collectionLocation),
    m_state(State::Scanning),
    m_info(nullptr),
    m_button(nullptr),
    m_database(db),
    m_gotPhotos(false),
    m_gotDBPhotos(false)
{
    m_info = new QLabel(this);
    m_button = new QPushButton(this);

    connect(m_button, &QPushButton::clicked, this, &CollectionDirScanDialog::buttonPressed);
    connect(&m_collector, &PhotosCollector::finished, this, &CollectionDirScanDialog::scanDone);
    connect(this, &CollectionDirScanDialog::analyze, this, &CollectionDirScanDialog::performAnalysis, Qt::QueuedConnection);

    // main layout
    QVBoxLayout* l = new QVBoxLayout(this);
    l->addWidget(m_info);
    l->addWidget(m_button);

    //
    scan();
}


CollectionDirScanDialog::~CollectionDirScanDialog()
{

}


void CollectionDirScanDialog::buttonPressed()
{
    if (m_state == State::Done || m_state == State::Canceled)
        accept();
    else
    {
        m_state = State::Canceled;
        m_collector.stop();

        updateGui();
    }
}


void CollectionDirScanDialog::scanDone()
{
    m_gotPhotos = true;

    checkIfReady();
}


void CollectionDirScanDialog::performAnalysis()
{
    m_state = State::Analyzing;
    updateGui();

    for(const IPhotoInfo::Ptr& photo: m_dbPhotos)
    {
        const QString path = photo->getPath();

        auto it = m_photosFound.find(path);

        if (it != m_photosFound.end())
            m_photosFound.erase(it);
    }

    // now m_photosFound contains only photos which are not in db
}


void CollectionDirScanDialog::scan()
{
    m_state = State::Scanning;
    updateGui();

    // collect photos from disk
    using namespace std::placeholders;
    auto disk_callback = std::bind(&CollectionDirScanDialog::gotPhoto, this, _1);

    m_collector.collect(m_collectionLocation, disk_callback);

    // collect photos from db
    auto db_callback = std::bind(&CollectionDirScanDialog::gotExistingPhotos, this, _1);
    auto task = std::make_unique<ListExistingPhotos>(db_callback);
    
    m_database->exec( std::move(task) );
}


void CollectionDirScanDialog::checkIfReady()
{
    if (m_gotPhotos && m_gotDBPhotos)
        emit analyze();
}


void CollectionDirScanDialog::gotPhoto(const QString& path)
{
    m_photosFound.insert(path);
}


void CollectionDirScanDialog::gotExistingPhotos(const IPhotoInfo::List& photos)
{
    m_dbPhotos = photos;
    m_gotDBPhotos = true;

    checkIfReady();
}


void CollectionDirScanDialog::updateGui()
{
    switch(m_state)
    {
        case State::Canceled:
            m_info->setText(tr("Collection scan canceled"));
            m_button->setText(tr("Close"));
            break;

        case State::Scanning:
            m_info->setText(tr("Collection scan in progress"));
            m_button->setText(tr("Cancel"));
            break;

        case State::Analyzing:
            m_info->setText(tr("Searching for new photos"));
            m_button->setText(tr("Cancel"));
            break;

        case State::Done:
            m_info->setText(tr("Done"));
            m_button->setText(tr("Close"));
            break;
    }
}
