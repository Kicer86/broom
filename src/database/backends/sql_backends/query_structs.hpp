
#ifndef DATABASE_QUERY_STRUCTS_HPP
#define DATABASE_QUERY_STRUCTS_HPP

#include <memory>
#include <deque>

#include <QString>

#include <utils/data_ptr.hpp>

#include "sql_backend_base_export.h"

class QStringList;

namespace Database
{
    class SQL_BACKEND_BASE_EXPORT InsertQueryData
    {
        public:
            enum class Value
            {
                Null,
                CurrentTime,
            };

            InsertQueryData(const char* name);
            InsertQueryData(const InsertQueryData &) = default;
            virtual ~InsertQueryData();

            //define common columns
            template<typename Column, typename... V>
            void setColumns(const Column& column, const V&... columns)
            {
                addColumn(column);
                setColumns(columns...);
            }

            //set values for common columns
            template<typename Value, typename... V>
            void setValues(const Value& value, const V&... values)
            {
                addValue(value);
                setValues(values...);
            }

            const QString& getName() const;
            const QStringList& getColumns() const;
            const QStringList& getValues() const;

        private:
            struct Data;
            ol::data_ptr<Data> m_data;

            void addColumn(const QString &);
            void addValue(const QString &);
            void addValue(int);
            void addValue(Value);

            //finish variadic templates
            void setColumns();
            void setValues();
    };


    class SQL_BACKEND_BASE_EXPORT UpdateQueryData: public InsertQueryData
    {
        public:
            UpdateQueryData(const char* name);
            explicit UpdateQueryData(const InsertQueryData &);

            //update where c == v
            void setCondition(const QString& c, const QString& v);
            const std::pair<QString, QString>& getCondition() const;

        private:
            std::pair<QString, QString> m_condition;
    };
}

#endif

