/*
 * Evaluator of search expressions
 * Copyright (C) 2016  Michał Walenciak <Kicer86@gmail.com>
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

#ifndef SEARCHEXPRESSIONEVALUATOR_HPP
#define SEARCHEXPRESSIONEVALUATOR_HPP

#include <vector>

#include <QString>

#include "core_export.h"


class CORE_EXPORT SearchExpressionEvaluator
{
    public:
        struct Filter
        {
            QString m_value;
            bool m_exact;
        };

        SearchExpressionEvaluator();
        SearchExpressionEvaluator(const SearchExpressionEvaluator &) = delete;

        SearchExpressionEvaluator& operator=(const SearchExpressionEvaluator &) = delete;

        std::vector<Filter> evaluate(const QString& input) const;

    private:
};

#endif // SEARCHEXPRESSIONEVALUATOR_HPP
