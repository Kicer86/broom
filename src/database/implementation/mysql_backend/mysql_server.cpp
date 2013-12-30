/*
 * A class for managing mysql server process
 * Copyright (C) 2013  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "mysql_server.hpp"

#include <fstream>

#include <QProcess>

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include "configuration/configurationfactory.hpp"
#include "configuration/iconfiguration.hpp"
#include "configuration/entrydata.hpp"
#include "system/system.hpp"

#include "databasebuilder.hpp"

namespace
{
    const char* MySQL_daemon = "Database::Backend::MySQL::Server";

    const char* MySQL_config =
    "#                                                                                            "
    "# Global Akonadi MySQL server settings,                                                      "
    "# These settings can be adjusted using $HOME/.config/akonadi/mysql-local.conf                "
    "#                                                                                            "
    "# Based on advice by Kris Köhntopp <kris@mysql.com>                                          "
    "#                                                                                            "
    "[mysqld]                                                                                     "

    "# strict query parsing/interpretation                                                        "
    "# TODO: make Akonadi work with those settings enabled                                        "
    "# sql_mode=strict_trans_tables,strict_all_tables,strict_error_for_division_by_zero,no_auto_create_user,no_auto_value_on_zero,no_engine_substitution,no_zero_date,no_zero_in_date,only_full_group_by,pipes_as_concat"
    "# sql_mode=strict_trans_tables                                                               "

    "# DEBUGGING:"
    "# log all queries, useful for debugging but generates an enormous amount of data"
    "# log=mysql.full"
    "# log queries slower than n seconds, log file name relative to datadir (for debugging only)"
    "# log_slow_queries=mysql.slow"
    "# long_query_time=1"
    "# log queries not using indices, debug only, disable for production use"
    "# log_queries_not_using_indexes=1"
    "#"
    "# mesure database size and adjust innodb_buffer_pool_size"
    "# SELECT sum(data_length) as bla, sum(index_length) as blub FROM information_schema.tables WHERE table_schema not in (\"mysql\", \"information_schema\");"

    "# NOTES:"
    "# Keep Innob_log_waits and keep Innodb_buffer_pool_wait_free small (see show global status like \"inno%\", show global variables)"

    "#expire_logs_days=3"

    "#sync_bin_log=0"

    "# Use UTF-8 encoding for tables"
    "character_set_server=utf8"
    "collation_server=utf8_general_ci"

    "# use InnoDB for transactions and better crash recovery"
    "default_storage_engine=innodb"

    "# memory pool InnoDB uses to store data dictionary information and other internal data structures (default:1M)"
    "# Deprecated in MySQL >= 5.6.3"
    "innodb_additional_mem_pool_size=1M"

    "# memory buffer InnoDB uses to cache data and indexes of its tables (default:128M)"
    "# Larger values means less I/O"
    "innodb_buffer_pool_size=80M"

    "# Create a .ibd file for each table (default:0)"
    "innodb_file_per_table=1"

    "# Write out the log buffer to the log file at each commit (default:1)"
    "innodb_flush_log_at_trx_commit=2"

    "# Buffer size used to write to the log files on disk (default:1M for builtin, 8M for plugin)"
    "# larger values means less I/O"
    "innodb_log_buffer_size=1M"

    "# Size of each log file in a log group (default:5M) larger means less I/O but more time for recovery."
    "innodb_log_file_size=64M"

    "# # error log file name, relative to datadir (default:hostname.err)"
    "log_error=mysql.err"

    "# print warnings and connection errors (default:1)"
    "log_warnings=2"

    "# Convert table named to lowercase"
    "lower_case_table_names=1"

    "# Maximum size of one packet or any generated/intermediate string. (default:1M)"
    "max_allowed_packet=32M"

    "# Maximum simultaneous connections allowed (default:100)"
    "max_connections=256"

    "# The two options below make no sense with prepared statements and/or transactions"
    "# (make sense when having the same query multiple times)"

    "# Memory allocated for caching query results (default:0 (disabled))"
    "query_cache_size=0"

    "# Do not cache results (default:1)"
    "query_cache_type=0"

    "# Do not use the privileges mechanisms"
    "skip_grant_tables"

    "# Do not listen for TCP/IP connections at all"
    "skip_networking"

    "# The number of open tables for all threads. (default:64)"
    "table_open_cache=200"

    "# How many threads the server should cache for reuse (default:0)"
    "thread_cache_size=3"

    "# wait 365d before dropping the DB connection (default:8h)"
    "wait_timeout=31536000"

    "[client]"
    "default-character-set=utf8";

    struct MySqlServerInit
    {

        MySqlServerInit()
        {
            std::shared_ptr<IConfiguration> config = ConfigurationFactory::get();

            const std::string configuration_xml =
                "<configuration>                                        "
                "    <keys>                                             "
                "        <key name='" + std::string(MySQL_daemon) + "' />            "
                "    </keys>                                            "
                "</configuration>                                       ";

            config->useXml(configuration_xml);

        }

    } init;
}


MySqlServer::MySqlServer(): m_serverProcess(new QProcess)
{

}



MySqlServer::~MySqlServer()
{

}


bool MySqlServer::run_server()
{
    bool status = false;

    //get path to server
    std::shared_ptr<IConfiguration> config = ConfigurationFactory::get();

    boost::optional<Configuration::EntryData> daemonPath = config->findEntry(MySQL_daemon);

    std::string path;
    if (daemonPath)
        path = daemonPath->value();
    else
    {
        path = System::findProgram("mysqld");

        Configuration::EntryData mysqldPath(MySQL_daemon, path);
        config->addEntry(mysqldPath);
    }

    if (path.empty() == false)
    {
        boost::optional<Configuration::EntryData> dbPath = config->findEntry(Database::databaseLocation);

        if (dbPath)
        {
            const std::string configDir = dbPath->value() + "mysql.conf";

            if (boost::filesystem::exists(configDir) == false)
            {
                std::ofstream os(configDir);

                os << MySQL_config;
            }

            const std::string mysql_config  = "--defaults-file=" + configDir;
            const std::string mysql_datadir = "--datadir=" + dbPath->value() + "db_data";
            const std::string mysql_socket  = "--socket=" + dbPath->value() + "mysql.socket";

            QStringList args = { mysql_config.c_str(), mysql_datadir.c_str(), mysql_socket.c_str() };

            m_serverProcess->setProgram(path.c_str());
            m_serverProcess->setArguments(args);
            m_serverProcess->closeWriteChannel();
            m_serverProcess->start();

            status = m_serverProcess->waitForStarted();
        }
    }

    return status;
}

