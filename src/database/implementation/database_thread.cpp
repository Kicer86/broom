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

#include "database_thread.hpp"

#include <thread>
#include <memory>

#include <OpenLibrary/putils/ts_queue.hpp>

#include "ibackend.hpp"
#include "iphoto_info_cache.hpp"
#include "photo_data.hpp"
#include "photo_info.hpp"
#include "photo_info_storekeeper.hpp"
#include "project_info.hpp"


namespace
{
    struct IThreadVisitor;
    struct InitTask;
    struct InsertPhotosTask;
    struct InsertTagTask;
    struct UpdateTask;
    struct GetAllPhotosTask;
    struct GetPhotoTask;
    struct GetPhotosTask;
    struct ListTagsTask;
    struct ListTagsTask2;
    struct ListTagValuesTask;
    struct ListTagValuesTask2;
    struct AnyPhotoTask;
    struct DropPhotosTask;
    struct PerformActionTask;

    struct IThreadTask
    {
        virtual ~IThreadTask() {}
        virtual void visitMe(IThreadVisitor *) = 0;
    };

    struct ThreadBaseTask: IThreadTask
    {
        ThreadBaseTask() {}
        virtual ~ThreadBaseTask() {}
    };

    struct IThreadVisitor
    {
        virtual ~IThreadVisitor() {}

        virtual void visit(InitTask *) = 0;
        virtual void visit(InsertPhotosTask *) = 0;
        virtual void visit(UpdateTask *) = 0;
        virtual void visit(InsertTagTask *) = 0;
        virtual void visit(GetAllPhotosTask *) = 0;
        virtual void visit(GetPhotoTask *) = 0;
        virtual void visit(GetPhotosTask *) = 0;
        virtual void visit(ListTagsTask *) = 0;
        virtual void visit(ListTagsTask2 *) = 0;
        virtual void visit(ListTagValuesTask *) = 0;
        virtual void visit(ListTagValuesTask2 *) = 0;
        virtual void visit(AnyPhotoTask *) = 0;
        virtual void visit(DropPhotosTask *) = 0;
        virtual void visit(PerformActionTask *) = 0;
    };

    struct InitTask: ThreadBaseTask
    {
        InitTask(std::unique_ptr<Database::AInitTask>&& task, const Database::ProjectInfo& prjInfo):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_prjInfo(prjInfo)
        {

        }

        virtual ~InitTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AInitTask> m_task;
        Database::ProjectInfo m_prjInfo;
    };

    struct InsertPhotosTask: ThreadBaseTask
    {
        InsertPhotosTask(const std::set<QString>& paths, const std::function<void(bool)>& callback):
            ThreadBaseTask(),
            m_paths(paths),
            m_callback(callback)
        {

        }

        virtual ~InsertPhotosTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::set<QString> m_paths;
        std::function<void(bool)> m_callback;
    };

    struct UpdateTask: ThreadBaseTask
    {
        UpdateTask(std::unique_ptr<Database::AStorePhotoTask>&& task, const IPhotoInfo::Ptr& photo):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_photoInfo(photo)
        {

        }

        virtual ~UpdateTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AStorePhotoTask> m_task;
        IPhotoInfo::Ptr m_photoInfo;
    };


    struct InsertTagTask: ThreadBaseTask
    {
        InsertTagTask(std::unique_ptr<Database::AStoreTagTask>&& task, const TagNameInfo& tagInfo):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_tagInfo(tagInfo)
        {
        }

        virtual ~InsertTagTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AStoreTagTask> m_task;
        TagNameInfo m_tagInfo;
    };


    struct GetAllPhotosTask: ThreadBaseTask
    {
        GetAllPhotosTask(std::unique_ptr<Database::AGetPhotosTask>&& task):
            ThreadBaseTask(),
            m_task(std::move(task))
        {

        }

        virtual ~GetAllPhotosTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AGetPhotosTask> m_task;
    };


    struct GetPhotoTask: ThreadBaseTask
    {
        GetPhotoTask(std::unique_ptr<Database::AGetPhotoTask>&& task, const Photo::Id& id):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_id(id)
        {

        }

        virtual ~GetPhotoTask() {}
        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AGetPhotoTask> m_task;
        Photo::Id m_id;
    };

    struct GetPhotosTask: ThreadBaseTask
    {
        GetPhotosTask(std::unique_ptr<Database::AGetPhotosTask>&& task, const std::deque<Database::IFilter::Ptr>& filter):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_filter(filter)
        {

        }

        virtual ~GetPhotosTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AGetPhotosTask> m_task;
        std::deque<Database::IFilter::Ptr> m_filter;
    };

    struct ListTagsTask: ThreadBaseTask
    {
        ListTagsTask(std::unique_ptr<Database::AListTagsTask>&& task):
            ThreadBaseTask(),
            m_task(std::move(task))
        {

        }

        virtual ~ListTagsTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AListTagsTask> m_task;
    };

    struct ListTagsTask2: ThreadBaseTask
    {
        ListTagsTask2(const std::function< void(const std::deque<TagNameInfo> &) > & callback):
            ThreadBaseTask(),
            m_callback(callback)
        {

        }

        virtual ~ListTagsTask2() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::function< void(const std::deque<TagNameInfo> &) > m_callback;
    };

    struct ListTagValuesTask: ThreadBaseTask
    {
        ListTagValuesTask(std::unique_ptr<Database::AListTagValuesTask>&& task, const TagNameInfo& info, const std::deque<Database::IFilter::Ptr>& filter):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_info(info),
            m_filter(filter)
        {

        }

        virtual ~ListTagValuesTask() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AListTagValuesTask> m_task;
        TagNameInfo m_info;
        std::deque<Database::IFilter::Ptr> m_filter;
    };

    struct ListTagValuesTask2: ThreadBaseTask
    {
        ListTagValuesTask2(const TagNameInfo& info,
                           const std::deque<Database::IFilter::Ptr>& filter,
                           const Database::IDatabase::Callback<const TagNameInfo &, const std::deque<QVariant> &> & callback):
        ThreadBaseTask(),
        m_callback(callback),
        m_info(info),
        m_filter(filter)
        {

        }

        virtual ~ListTagValuesTask2() {}

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        const Database::IDatabase::Callback<const TagNameInfo &, const std::deque<QVariant> &> m_callback;
        TagNameInfo m_info;
        std::deque<Database::IFilter::Ptr> m_filter;
    };

    struct AnyPhotoTask: ThreadBaseTask
    {
        AnyPhotoTask(std::unique_ptr<Database::AGetPhotosCount>&& task, const std::deque<Database::IFilter::Ptr>& filter):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_filter(filter)
        {

        }

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::AGetPhotosCount> m_task;
        std::deque<Database::IFilter::Ptr> m_filter;
    };

    struct DropPhotosTask: ThreadBaseTask
    {
        DropPhotosTask(std::unique_ptr<Database::ADropPhotosTask>&& task, const std::deque<Database::IFilter::Ptr>& filter):
            ThreadBaseTask(),
            m_task(std::move(task)),
            m_filter(filter)
        {

        }

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::unique_ptr<Database::ADropPhotosTask> m_task;
        std::deque<Database::IFilter::Ptr> m_filter;
    };

    struct PerformActionTask: ThreadBaseTask
    {
        PerformActionTask(const std::deque<Database::IFilter::Ptr>& filter, const std::deque<Database::IAction::Ptr>& action):
            ThreadBaseTask(),
            m_filter(filter),
            m_action(action)
        {
        }

        virtual void visitMe(IThreadVisitor* visitor) { visitor->visit(this); }

        std::deque<Database::IFilter::Ptr> m_filter;
        std::deque<Database::IAction::Ptr> m_action;
    };

    struct Executor: IThreadVisitor, Database::ADatabaseSignals
    {
        Executor( std::unique_ptr<Database::IBackend>&& backend, PhotoInfoStorekeeper* storekeeper):
            m_tasks(1024),
            m_backend( std::move(backend) ),
            m_cache(nullptr),
            m_storekeeper(storekeeper)
        {

        }

        Executor(const Executor &) = delete;
        Executor& operator=(const Executor &) = delete;

        virtual ~Executor() {}

        void set(Database::IPhotoInfoCache* cache)
        {
            m_cache = cache;
        }

        virtual void visit(InitTask* task) override
        {
            Database::BackendStatus status = m_backend->init(task->m_prjInfo);
            task->m_task->got(status);
        }

        virtual void visit(InsertPhotosTask* task) override
        {
            for(const QString& path: task->m_paths)
            {
                const bool status = insertPhoto(path);
                if (task->m_callback)
                    task->m_callback(status);
            }
        }

        virtual void visit(UpdateTask* task) override
        {
            Photo::Data data = task->m_photoInfo->data();
            const bool status = m_backend->update(data);
            task->m_task->got(status);

            emit photoModified(task->m_photoInfo);
        }

        virtual void visit(InsertTagTask* task) override
        {
            const bool status = m_backend->update(task->m_tagInfo);
            task->m_task->got(status);
        }

        virtual void visit(GetAllPhotosTask* task) override
        {
            auto photos = m_backend->getAllPhotos();
            IPhotoInfo::List photosList;

            for(const Photo::Id& id: photos)
                photosList.push_back(getPhotoFor(id));

            task->m_task->got(photosList);
        }

        virtual void visit(GetPhotoTask* task) override
        {
            auto photo = getPhotoFor(task->m_id);

            task->m_task->got(photo);
        }

        virtual void visit(GetPhotosTask* task) override
        {
            auto photos = m_backend->getPhotos(task->m_filter);
            IPhotoInfo::List photosList;

            for(const Photo::Id& id: photos)
                photosList.push_back(getPhotoFor(id));

            task->m_task->got(photosList);
        }

        virtual void visit(ListTagsTask* task) override
        {
            auto result = m_backend->listTags();
            task->m_task->got(result);
        }

        virtual void visit(ListTagsTask2* task) override
        {
            auto result = m_backend->listTags();
            task->m_callback(result);
        }

        virtual void visit(ListTagValuesTask* task) override
        {
            auto result = m_backend->listTagValues(task->m_info, task->m_filter);
            task->m_task->got(result);
        }

        virtual void visit(ListTagValuesTask2* task) override
        {
            auto result = m_backend->listTagValues(task->m_info, task->m_filter);
            task->m_callback(task->m_info, result);
        }

        virtual void visit(AnyPhotoTask* task) override
        {
            auto result = m_backend->getPhotosCount(task->m_filter);
            task->m_task->got(result);
        }

        virtual void visit(DropPhotosTask* task) override
        {
            auto photos = m_backend->dropPhotos(task->m_filter);

            task->m_task->got(photos);

            emit photosRemoved(photos);
        }

        virtual void visit(PerformActionTask* task) override
        {
            m_backend->perform(task->m_filter, task->m_action);
        }

        void begin()
        {
            for(;;)
            {
                ol::Optional< std::unique_ptr<ThreadBaseTask> > task = m_tasks.pop();

                if (task)
                {
                    ThreadBaseTask* baseTask = task->get();
                    baseTask->visitMe(this);
                }
                else
                    break;
            }
        }

        void stop()
        {
            m_tasks.stop();
        }

        void closeConnections()
        {
            m_backend->closeConnections();
        }

        IPhotoInfo::Ptr getPhotoFor(const Photo::Id& id)
        {
            IPhotoInfo::Ptr photoPtr = m_cache->find(id);

            if (photoPtr.get() == nullptr)
            {
                const Photo::Data photoData = m_backend->getPhoto(id);

                photoPtr = std::make_shared<PhotoInfo>(photoData);

                m_cache->introduce(photoPtr);
                m_storekeeper->photoInfoConstructed(photoPtr);
            }

            return photoPtr;
        }

        bool insertPhoto(const QString& path)
        {
            Photo::Data data;
            data.path = path;
            data.flags[Photo::FlagsE::StagingArea] = 1;

            const bool status = m_backend->addPhoto(data);

            if (status)
            {
                IPhotoInfo::Ptr photoInfo = getPhotoFor(data.id);
                emit photoAdded(photoInfo);
            }

            return status;
        }

        void addTask(std::unique_ptr<ThreadBaseTask>&& task)
        {
            m_tasks.push(std::move(task));
        }

        private:
            ol::TS_Queue<std::unique_ptr<ThreadBaseTask>> m_tasks;
            std::unique_ptr<Database::IBackend> m_backend;
            Database::IPhotoInfoCache* m_cache;
            PhotoInfoStorekeeper* m_storekeeper;
    };

}


namespace Database
{

    struct DatabaseThread::Impl
    {
        Impl( std::unique_ptr<IBackend>&& backend):
            m_cache(nullptr),
            m_storekeeper(),
            m_executor( std::move(backend), &m_storekeeper),
            m_thread(),
            m_working(true)
        {
            m_thread = std::thread(&Executor::begin, &m_executor);
        }

        //store task to be executed by thread
        void addTask(ThreadBaseTask* task)
        {
            assert(m_working);
            m_executor.addTask( std::move(std::unique_ptr<ThreadBaseTask>(task)) );
        }

        void stopExecutor()
        {
            if (m_working)
            {
                m_working = false;
                m_executor.stop();

                assert(m_thread.joinable());
                m_thread.join();

                m_executor.closeConnections();
            }
        }

        std::unique_ptr<IPhotoInfoCache> m_cache;
        PhotoInfoStorekeeper m_storekeeper;
        Executor m_executor;
        std::thread m_thread;
        bool m_working;
    };


    DatabaseThread::DatabaseThread( std::unique_ptr<IBackend>&& backend ):
        m_impl(nullptr)
    {
        m_impl = std::make_unique<Impl>( std::move(backend));
        m_impl->m_storekeeper.setDatabase(this);
    }


    DatabaseThread::~DatabaseThread()
    {
        //terminate thread
        closeConnections();
    }


    void DatabaseThread::closeConnections()
    {
        m_impl->stopExecutor();
    }



    void DatabaseThread::set( std::unique_ptr<IPhotoInfoCache>&& cache)
    {
        m_impl->m_cache = std::move(cache);
        m_impl->m_executor.set(m_impl->m_cache.get());
        m_impl->m_storekeeper.setCache(m_impl->m_cache.get());
    }


    ADatabaseSignals* DatabaseThread::notifier()
    {
        return &m_impl->m_executor;
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AInitTask>&& db_task, const Database::ProjectInfo& prjInfo)
    {
        InitTask* task = new InitTask(std::move(db_task), prjInfo);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AStorePhotoTask>&& db_task, const IPhotoInfo::Ptr& photo)
    {
        UpdateTask* task = new UpdateTask(std::move(db_task), photo);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<AStoreTagTask>&& db_task, const TagNameInfo& tagNameInfo)
    {
        InsertTagTask* task = new InsertTagTask(std::move(db_task), tagNameInfo);
        m_impl->addTask(task);
    }


    void DatabaseThread::store(const std::set<QString>& paths, const std::function<void(bool)>& callback)
    {
        InsertPhotosTask* task = new InsertPhotosTask(paths, callback);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AListTagsTask>&& db_task)
    {
        ListTagsTask* task = new ListTagsTask(std::move(db_task));
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AGetPhotoTask>&& db_task, const Photo::Id& id)
    {
        GetPhotoTask* task = new GetPhotoTask(std::move(db_task), id);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AGetPhotosTask>&& db_task, const std::deque<Database::IFilter::Ptr>& filter)
    {
        GetPhotosTask* task = new GetPhotosTask(std::move(db_task), filter);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr< Database::AGetPhotosTask >&& db_task)
    {
        GetAllPhotosTask* task = new GetAllPhotosTask(std::move(db_task));
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AListTagValuesTask>&& db_task, const TagNameInfo& info)
    {
        ListTagValuesTask* task = new ListTagValuesTask(std::move(db_task), info, std::deque<IFilter::Ptr>());
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<Database::AListTagValuesTask>&& db_task, const TagNameInfo& info, const std::deque<Database::IFilter::Ptr>& filter)
    {
        ListTagValuesTask* task = new ListTagValuesTask(std::move(db_task), info, filter);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<AGetPhotosCount>&& db_task, const std::deque<IFilter::Ptr>& filters)
    {
        AnyPhotoTask* task = new AnyPhotoTask(std::move(db_task), filters);
        m_impl->addTask(task);
    }


    void DatabaseThread::listTagNames( const std::function< void(const std::deque<TagNameInfo> &) >& callback)
    {
        ListTagsTask2* task = new ListTagsTask2(callback);
        m_impl->addTask(task);
    }


    void DatabaseThread::listTagValues( const TagNameInfo& info, const Callback<const TagNameInfo &, const std::deque<QVariant> &> & callback)
    {
        ListTagValuesTask2* task = new ListTagValuesTask2(info, std::deque<IFilter::Ptr>(), callback);
        m_impl->addTask(task);
    }


    void DatabaseThread::perform(const std::deque<IFilter::Ptr>& filters, const std::deque<IAction::Ptr>& actions)
    {
        PerformActionTask* task = new PerformActionTask(filters, actions);
        m_impl->addTask(task);
    }


    void DatabaseThread::exec(std::unique_ptr<ADropPhotosTask>&& db_task , const std::deque<IFilter::Ptr>& filters)
    {
        DropPhotosTask* task = new DropPhotosTask(std::move(db_task), filters);
        m_impl->addTask(task);
    }

}
