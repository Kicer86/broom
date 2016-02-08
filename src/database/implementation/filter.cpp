/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "filter.hpp"

namespace Database
{

    IFilter::~IFilter()
    {

    }

    
    IFilterVisitor::~IFilterVisitor()
    {

    }


    EmptyFilter::~EmptyFilter()
    {

    }


    FilterPhotosWithTag::FilterPhotosWithTag(): tagName(), tagValue()
    {

    }


    FilterPhotosWithTag::~FilterPhotosWithTag()
    {

    }


    FilterPhotosWithFlags::FilterPhotosWithFlags(): flags(), mode(Mode::And)
    {

    }


    FilterPhotosWithFlags::~FilterPhotosWithFlags()
    {

    }


    FilterPhotosWithSha256::FilterPhotosWithSha256(): sha256("")
    {

    }


    FilterPhotosWithSha256::~FilterPhotosWithSha256()
    {

    }


    FilterNotMatchingFilter::FilterNotMatchingFilter(): filter()
    {

    }


    FilterNotMatchingFilter::~FilterNotMatchingFilter()
    {

    }


    FilterPhotosWithId::FilterPhotosWithId(): filter()
    {

    }


    FilterPhotosWithId::~FilterPhotosWithId()
    {

    }


    FilterPhotosMatchingExpression::FilterPhotosMatchingExpression(const QString& expr): expression(expr)
    {

    }


    FilterPhotosMatchingExpression::~FilterPhotosMatchingExpression()
    {

    }


    FilterPhotosWithPath::FilterPhotosWithPath(const QString& p): path(p)
    {

    }


    FilterPhotosWithPath::~FilterPhotosWithPath()
    {

    }

}
