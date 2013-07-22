
#include "photo_crawler.hpp"

#include "ifile_system_scanner.hpp"
#include "ianalyzer.hpp"

struct PhotoCrawler::Impl
{
	Impl(IFileSystemScanner *scanner, IAnalyzer *analyzer): m_scanner(scanner), m_analyzer(analyzer) {}
	virtual ~Impl() {}

	std::unique_ptr<IFileSystemScanner> m_scanner;
	std::unique_ptr<IAnalyzer> m_analyzer;
};


PhotoCrawler::PhotoCrawler(IFileSystemScanner *scanner, IAnalyzer *analyzer): m_impl(new Impl(scanner, analyzer))
{

}


PhotoCrawler::~PhotoCrawler()
{

}


std::vector<std::string> PhotoCrawler::crawl(const std::string &path)
{
    std::vector<std::string> files = m_impl->m_scanner->getFilesFor(path);

    std::vector<std::string> mediaFiles;
    for(auto file: files)
        if (m_impl->m_analyzer->isImage(file))
            mediaFiles.push_back(file);
	
	return mediaFiles;
}


void PhotoCrawler::setRules(const Rules &)
{

}
