
#ifndef DATABASE_MYSQL_BACKEND_HPP
#define DATABASE_MYSQL_BACKEND_HPP

#include <memory>

#include <database/idatabase.hpp>
#include <database/backends/sql_backends/sql_backend.hpp>
#include <database/backends/sql_backends/generic_sql_query_constructor.hpp>
#include <database/implementation/ibackend_qt_interface.hpp>

#include "database_mysql_backend_export.h"

namespace Database
{

    class MySqlBackend final: public ASqlBackend, GenericSqlQueryConstructor
    {
        public:
            MySqlBackend();
            virtual ~MySqlBackend();

        private:
            // ASqlBackend:
            virtual BackendStatus prepareDB(const ProjectInfo &) override;
            virtual QString prepareCreationQuery(const QString& name, const QString& columns) const override;
            virtual QString prepareColumnDescription(const ColDefinition &) const override;
            virtual const ISqlQueryConstructor* getQueryConstructor() const override;
            virtual void set(IConfiguration *) override;
            virtual void set(ILoggerFactory *) override;

            // GenericSqlQueryConstructor:
            virtual SqlQuery insertOrUpdate(const InsertQueryData&) const override;

            struct Data;
            std::unique_ptr<Data> m_data;
    };


    class DATABASE_MYSQL_BACKEND_EXPORT MySqlPlugin final: public QObject, public IPlugin
    {
            Q_OBJECT
            Q_PLUGIN_METADATA(IID DatabasePluginInterface_iid FILE "mysql_backend.json")
            Q_INTERFACES(Database::IPlugin)   //'Database' namespace is obligatory

        public:
            MySqlPlugin();
            virtual ~MySqlPlugin();

            virtual std::unique_ptr<IBackend> constructBackend() override;
            virtual QString backendName() const override;
            virtual ProjectInfo initPrjDir(const QString&) const override;
            virtual QLayout* buildDBOptions() override;
            virtual char simplicity() const override;
    };


}

#endif
