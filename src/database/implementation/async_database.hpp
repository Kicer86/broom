/*
 * Database thread.
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

#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include <deque>

#include "idatabase.hpp"
#include "ibackend.hpp"

namespace Database
{

    class AsyncDatabase: public IDatabase
    {
        public:
            AsyncDatabase (std::unique_ptr<IBackend> &&);
            AsyncDatabase (const AsyncDatabase &) = delete;
            virtual ~AsyncDatabase();

            AsyncDatabase& operator=(const AsyncDatabase &) = delete;

            void set(std::unique_ptr<IPhotoInfoCache> &&);

            virtual ADatabaseSignals* notifier() override;

            virtual void update(const IPhotoInfo::Ptr &) override;
            virtual void store(const std::set< QString >&, const Photo::FlagValues &, const Callback<const std::vector<Photo::Id> &>&) override;
            virtual void createGroup(const Photo::Id & , const Callback<Group::Id> &) override;

            virtual void countPhotos(const std::deque<IFilter::Ptr> &, const Callback<int> &) override;
            virtual void getPhotos(const std::vector<Photo::Id> &, const Callback<const std::deque<IPhotoInfo::Ptr> &> &) override;
            virtual void listTagNames( const Callback<const std::deque<TagNameInfo> &> & ) override;
            virtual void listTagValues( const TagNameInfo&, const Callback<const TagNameInfo &, const std::deque<TagValue> &> &) override;
            virtual void listTagValues( const TagNameInfo&, const std::deque<IFilter::Ptr> &, const Callback<const TagNameInfo &, const std::deque<TagValue> &> &) override;
            virtual void listPhotos(const std::deque<IFilter::Ptr> &, const Callback<const IPhotoInfo::List &> &) override;

            virtual void performCustomAction(const std::function<void(IBackendOperator *)> &) override;

            virtual void init(const ProjectInfo &, const Callback<const BackendStatus &> &) override;
            virtual void closeConnections() override;

        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    };

}

#endif // DATABASETHREAD_H