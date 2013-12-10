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


#include "databasebuilder.hpp"

#include <assert.h>

#include <memory>
#include <fstream>

#include "configuration/configurationfactory.hpp"
#include "configuration/iconfiguration.hpp"
#include "configuration/entrydata.hpp"

#include "memorydatabase.hpp"
#include "ifs.hpp"
#include "implementation/backend.hpp"

namespace Database
{

    namespace
    {
        std::unique_ptr<IFrontend> defaultDatabase;
        
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
    }


    Builder::Builder()
    {

    }


    Builder::~Builder()
    {

    }


    IFrontend* Builder::get()
    {
        if (defaultDatabase.get() == nullptr)
        {   
            std::shared_ptr< ::IConfiguration > config = ConfigurationFactory::get();
            boost::optional<Configuration::EntryData> entry = config->findEntry(Configuration::configLocation);
            
            assert(entry.is_initialized());
            
            const std::string configPath = entry->value();
            const std::string dbPath = configPath + "/database";
            
            std::vector<Configuration::EntryData> entries = 
            {
                Configuration::EntryData("Database::Backend::DataLocation",  dbPath)
            };
            
            ConfigurationFactory::get()->registerEntries(entries);
            
            std::shared_ptr<IStreamFactory> fs(new StreamFactory);
            IFrontend *frontend = new MemoryDatabase(fs);
            
            defaultDatabase.reset(frontend);
            defaultDatabase->setBackend(std::shared_ptr<Database::IBackend>(new Database::PrimitiveBackend));
        }

        return defaultDatabase.get();
    }

}
