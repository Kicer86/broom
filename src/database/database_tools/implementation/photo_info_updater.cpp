
#include "photo_info_updater.hpp"

#include <assert.h>

#include <memory>

#include <QPixmap>
#include <QImage>
#include <qcryptographichash.h>

#include <core/task_executor.hpp>
#include <core/photos_manager.hpp>
#include <core/itagfeeder.hpp>
#include <core/tag.hpp>
#include <configuration/iconfiguration.hpp>
#include <configuration/constants.hpp>


struct UpdaterTask: ITaskExecutor::ITask
{
    UpdaterTask(PhotoInfoUpdater* updater): m_updater(updater)
    {
        m_updater->taskAdded(this);
    }

    virtual ~UpdaterTask()
    {
        m_updater->taskFinished(this);
    }

    UpdaterTask(const UpdaterTask &) = delete;
    UpdaterTask& operator=(const UpdaterTask &) = delete;

    PhotoInfoUpdater* m_updater;
};


namespace
{

    struct Sha256Assigner: UpdaterTask
    {
        Sha256Assigner(PhotoInfoUpdater* updater,
                    IPhotosManager* photosManager,
                    const IPhotoInfo::Ptr& photoInfo):
            UpdaterTask(updater),
            m_photoInfo(photoInfo),
            m_photosManager(photosManager)
        {
        }

        Sha256Assigner(const Sha256Assigner &) = delete;
        Sha256Assigner& operator=(const Sha256Assigner &) = delete;

        virtual std::string name() const override
        {
            return "Photo hash generation";
        }

        virtual void perform() override
        {
            const QByteArray data = m_photosManager->getPhoto(m_photoInfo);
            const QByteArray rawHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
            const QByteArray hexHash = rawHash.toHex();
            m_photoInfo->setSha256(hexHash);
        }

        IPhotoInfo::Ptr m_photoInfo;
        IPhotosManager* m_photosManager;
    };

}

struct TagsCollector: UpdaterTask
{
    TagsCollector(PhotoInfoUpdater* updater, const IPhotoInfo::Ptr& photoInfo): UpdaterTask(updater), m_photoInfo(photoInfo), m_tagFeederFactory(nullptr)
    {
    }

    TagsCollector(const TagsCollector &) = delete;
    TagsCollector& operator=(const TagsCollector &) = delete;

    void set(ITagFeederFactory* tagFeederFactory)
    {
        m_tagFeederFactory = tagFeederFactory;
    }

    virtual std::string name() const override
    {
        return "Photo tags collection";
    }

    virtual void perform() override
    {
        const QString& path = m_photoInfo->getPath();
        std::shared_ptr<ITagFeeder> feeder = m_tagFeederFactory->get();
        Tag::TagsList p_tags = feeder->getTagsFor(path);

        m_photoInfo->setTags(p_tags);
        m_photoInfo->markFlag(Photo::FlagsE::ExifLoaded, 1);
    }

    IPhotoInfo::Ptr m_photoInfo;
    ITagFeederFactory* m_tagFeederFactory;
};


PhotoInfoUpdater::PhotoInfoUpdater():
    m_tagFeederFactory(),
    m_taskQueue(),
    m_tasks(),
    m_tasksMutex(),
    m_finishedTask(),
    m_configuration(nullptr),
    m_photosManager(nullptr)
{

}


PhotoInfoUpdater::~PhotoInfoUpdater()
{

}


void PhotoInfoUpdater::updateSha256(const IPhotoInfo::Ptr& photoInfo)
{
    auto task = std::make_unique<Sha256Assigner>(this, m_photosManager, photoInfo);

    m_taskQueue->push(std::move(task));
}


void PhotoInfoUpdater::updateTags(const IPhotoInfo::Ptr& photoInfo)
{
    auto task = std::make_unique<TagsCollector>(this, photoInfo);
    task->set(&m_tagFeederFactory);

    m_taskQueue->push(std::move(task));
}


void PhotoInfoUpdater::set(ITaskExecutor* taskExecutor)
{
    m_taskQueue = taskExecutor->getCustomTaskQueue();
}


void PhotoInfoUpdater::set(IConfiguration* configuration)
{
    m_configuration = configuration;
}


void PhotoInfoUpdater::set(IPhotosManager* photosManager)
{
    m_photosManager = photosManager;
    m_tagFeederFactory.set(photosManager);
}


int PhotoInfoUpdater::tasksInProgress()
{
    return m_tasks.size();
}


void PhotoInfoUpdater::dropPendingTasks()
{
    if (m_taskQueue != nullptr)
        m_taskQueue->clear();
}


void PhotoInfoUpdater::waitForActiveTasks()
{
    std::unique_lock<std::mutex> lock(m_tasksMutex);
    m_finishedTask.wait(lock, [&]
    {
        return m_tasks.empty();
    });
}


void PhotoInfoUpdater::taskAdded(UpdaterTask* task)
{
    std::lock_guard<std::mutex> lock(m_tasksMutex);
    m_tasks.insert(task);
}


void PhotoInfoUpdater::taskFinished(UpdaterTask* task)
{
    {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        m_tasks.erase(task);
    }

    m_finishedTask.notify_one();
}
