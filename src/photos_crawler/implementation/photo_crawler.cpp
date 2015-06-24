
#include "photo_crawler.hpp"

#include <cassert>
#include <thread>

#include <QString>

#include "ifile_system_scanner.hpp"
#include "ianalyzer.hpp"

namespace
{

    struct FileNotifier: IFileNotifier
    {
        FileNotifier(IAnalyzer* analyzer, IMediaNotification* notifications): m_analyzer(analyzer), m_notifications(notifications) {}
        FileNotifier(const FileNotifier &) = delete;

        FileNotifier& operator=(const FileNotifier &) = delete;

        virtual void found(const QString& file) override
        {
            if (m_analyzer->isImage(file))
                m_notifications->found(file);
        }

        virtual void finished() override
        {
            m_notifications->finished();
        }

        IAnalyzer* m_analyzer;
        IMediaNotification* m_notifications;
    };

}


struct PhotoCrawler::Impl
{
    Impl(const std::shared_ptr<IFileSystemScanner>& scanner,
         const std::shared_ptr<IAnalyzer>& analyzer): m_scanner(scanner), m_analyzer(analyzer), m_thread() {}
    ~Impl()
    {
        releaseThread();
    }

    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

    std::shared_ptr<IFileSystemScanner> m_scanner;
    std::shared_ptr<IAnalyzer> m_analyzer;
    std::thread m_thread;

    void run(const QString& path, IMediaNotification* notifications)
    {
        releaseThread();
        
        m_thread = std::thread( [&]
        {
            this->thread(path, notifications);
        });
    }

    void releaseThread()
    {
        if (m_thread.joinable())
            m_thread.join();
    }

    //thread function
    void thread(const QString& path, IMediaNotification* notifications)
    {
        FileNotifier notifier(m_analyzer.get(), notifications);

        m_scanner->getFilesFor(path, &notifier);
    }
};


PhotoCrawler::PhotoCrawler(const std::shared_ptr<IFileSystemScanner>& scanner,
                           const std::shared_ptr<IAnalyzer>& analyzer): m_impl(new Impl(scanner, analyzer))
{

}


PhotoCrawler::~PhotoCrawler()
{

}


void PhotoCrawler::crawl(const QString& path, IMediaNotification* notifications)
{
    m_impl->run(path, notifications);
}


void PhotoCrawler::setRules(const Rules &)
{

}
