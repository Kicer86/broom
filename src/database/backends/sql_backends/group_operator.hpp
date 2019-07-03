/*
 * Class for performing operations on groups
 * Copyright (C) 2019  Michał Walenciak <Kicer86@gmail.com>
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
 */

#ifndef GROUPOPERATOR_HPP
#define GROUPOPERATOR_HPP

#include <QString>

#include <database/igroup_operator.hpp>


struct ILogger;

namespace Database
{
    struct IBackend;
    struct IGenericSqlQueryGenerator;
    struct ISqlQueryExecutor;

    class GroupOperator final: public IGroupOperator
    {
        public:
            GroupOperator(const QString &, const IGenericSqlQueryGenerator *, Database::ISqlQueryExecutor *, ILogger *, IBackend *);

            Group::Id addGroup(const Photo::Id &, Group::Type) const override;
            Photo::Id removeGroup(const Group::Id &) const override;
            Group::Type type(const Group::Id &) const override;
            std::vector<Photo::Id> membersOf(const Group::Id &) const override;

        private:
            QString m_connectionName;
            const IGenericSqlQueryGenerator* m_queryGenerator;
            ISqlQueryExecutor* m_executor;
            ILogger* m_logger;
            IBackend* m_backend;
    };
}

#endif // GROUPOPERATOR_HPP
