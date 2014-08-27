/*
    Factory for database
    Copyright (C) 2013  Michał Walenciak <MichalWalenciak@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "database_builder.hpp"

#include <assert.h>

#include <memory>
#include <fstream>
#include <map>

#include <configuration/configurationfactory.hpp>
#include <configuration/iconfiguration.hpp>
#include <configuration/entrydata.hpp>
#include <database_tools/photos_analyzer.hpp>

#include "ifs.hpp"
#include "iphoto_info_creator.hpp"
#include "plugin_loader.hpp"
#include "database_thread.hpp"
#include "photo_info_manager.hpp"
#include "idatabase_plugin.hpp"
#include "idatabase.hpp"
#include "implementation/photo_info.hpp"

//TODO: cleanup this file!

namespace Database
{

    const char* databaseLocation = "Database::Backend::DataLocation";

    namespace
    {

        struct StreamFactory: public IStreamFactory
        {
            virtual ~StreamFactory()
            {

            }

            virtual std::shared_ptr<std::iostream> openStream(const std::string &filename,
                    std::ios_base::openmode mode) override
            {
                auto stream = std::make_shared<std::fstream>();

                stream->open(filename.c_str(), mode);

                return stream;
            }
        };

        //class which initializes configuration with db entries
        struct ConfigurationInitializer: public Configuration::IInitializer
        {
            ConfigurationInitializer()
            {

            }

            virtual std::string getXml()
            {
                std::shared_ptr< ::IConfiguration > config = ConfigurationFactory::get();
                Optional<Configuration::EntryData> entry = config->findEntry(Configuration::configLocation);

                assert(entry.is_initialized());

                const std::string configPath = entry->value();
                const std::string dbPath = configPath + "/database";

                const std::string configuration_xml =
                    "<configuration>                                        "
                    "   <keys>                                              "
                    "   </keys>                                             "
                    "                                                       "
                    "   <defaults>                                          "
                    "       <key name='Database::Backend::DataLocation' value='" + dbPath + "' />"
                    "   </defaults>                                         "
                    "</configuration>                                       ";

                return configuration_xml;
            }
        };
    }

    struct Builder::Impl
    {

        //backend type
        enum Type
        {
            Main,
        };

        struct DatabaseObjects
        {
			DatabaseObjects(const std::shared_ptr<IDatabase> &database, 
							const std::shared_ptr<IBackend> &backend,
							const std::shared_ptr<IPhotoInfoManager> &manager,
                            const std::shared_ptr<PhotosAnalyzer> &analyzer
                           ) : m_database(database), m_backend(backend), m_photoManager(manager), m_photosAnalyzer(analyzer)
            {

            }
            
			~DatabaseObjects() {}
			//DatabaseObjects(const DatabaseObjects &) = delete;
			//DatabaseObjects& operator=(const DatabaseObjects &) = delete;

			std::shared_ptr<IDatabase> m_database;
			std::shared_ptr<IBackend> m_backend;
			std::shared_ptr<IPhotoInfoManager> m_photoManager;
            std::shared_ptr<PhotosAnalyzer> m_photosAnalyzer;
        };

        struct PhotoInfoCreator: Database::IPhotoInfoCreator
        {
            virtual IPhotoInfo::Ptr construct(const QString& path)
            {
                auto result = std::make_shared<PhotoInfo>(path);
                result->markStagingArea(true);                                //by default all new photos go to staging area.

                return result;
            }
        };

        std::map<Type, DatabaseObjects> m_backends;
        std::unique_ptr<IPlugin> plugin;
        std::shared_ptr<IBackend> defaultBackend;
        PluginLoader backendBuilder;
        ConfigurationInitializer configInitializer;
        PhotoInfoCreator photoInfoCreator;

        Impl(): m_backends(), plugin(), defaultBackend(), backendBuilder(), configInitializer(), photoInfoCreator()
        {}

        IPlugin* getPlugin()
        {
            if (plugin.get() == nullptr)
                plugin = backendBuilder.get();

            return plugin.get();
        }
    };


    Builder::Builder(): m_impl(new Impl)
    {

    }


    Builder::~Builder()
    {

    }


    Builder* Builder::instance()
    {
        static Builder builder;
        return &builder;
    }


    void Builder::initConfig()
    {
        std::shared_ptr< ::IConfiguration > config = ConfigurationFactory::get();
        config->registerInitializer(&m_impl->configInitializer);
    }


    IDatabase* Builder::get()
    {
        const char* dbType = "broom";

        auto backendIt = m_impl->m_backends.find(Impl::Main);

        if (backendIt == m_impl->m_backends.end())
        {
            std::shared_ptr<PhotoInfoManager> manager(new PhotoInfoManager);
            std::shared_ptr<IBackend> backend = m_impl->getPlugin()->constructBackend();
            std::shared_ptr<IDatabase> database(new DatabaseThread(backend.get()));
            std::shared_ptr<PhotosAnalyzer> analyzer(new PhotosAnalyzer);

            backend->setPhotoInfoManager(manager.get());
            backend->setPhotoInfoCreator(&m_impl->photoInfoCreator);
            manager->setDatabase(database.get());
            analyzer->setDatabase(database.get());

            Database::Task task = database->prepareTask(nullptr);

            const bool status = database->init(task, dbType);


            if (status)
            {
                auto insertIt = m_impl->m_backends.emplace( std::make_pair(Impl::Main,
                                                                           Impl::DatabaseObjects(database, backend, manager, analyzer)
                                                                          )
                                                          );
                backendIt = insertIt.first;
            }
        }

        return backendIt->second.m_database.get();
    }


    void Builder::closeAll()
    {
        for(auto& backend: m_impl->m_backends)
            backend.second.m_database->closeConnections();
    }

}
