/*
 * Class for reseting items sizes and positions.
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

#include "positions_reseter.hpp"

#include <QModelIndex>

#include "data.hpp"

PositionsReseter::PositionsReseter(Data* data): m_data(data)
{

}


PositionsReseter::~PositionsReseter()
{

}


void PositionsReseter::itemsAdded(const QModelIndex& parent, int last) const
{
    invalidateItemOverallRect(parent);

    const QModelIndex lastItem = parent.child(last, 0);
    invalidateSiblingsRect(lastItem);
}


void PositionsReseter::invalidateAll() const
{
    m_data->for_each( [&](const ModelIndexInfo& c_info)
    {
        ModelIndexInfo info = c_info;
        info.cleanRects();
        m_data->update(info);

        return true;
    });
}


void PositionsReseter::itemChanged(const QModelIndex& idx)
{
    invalidateItemOverallRect(idx);
}


void PositionsReseter::invalidateItemOverallRect(const QModelIndex& idx) const
{
    resetOverallRect(idx);

    if (idx != QModelIndex())                           //do not invalidate root's parent if it doesn't exist
    {
        //if 'this' becomes invalid, invalidate also its parent
        const QModelIndex parent = idx.parent();
        invalidateItemOverallRect(parent);
        invalidateSiblingsRect(idx);
    }
}


void PositionsReseter::invalidateSiblingsRect(const QModelIndex& idx) const
{
    if (idx.isValid())
    {
        int row = idx.row() + 1;

        for(QModelIndex sibling = idx.sibling(row, 0); sibling.isValid(); sibling = sibling.sibling(row++, 0) )
            resetRect(sibling);
    }
}


void PositionsReseter::invalidateChildrenRect(const QModelIndex& idx) const
{
    int r = 0;
    for(QModelIndex child = idx.child(0, 0); child.isValid(); child = idx.child(++r, 0))
        resetRect(child);
}


void PositionsReseter::resetRect(const QModelIndex& idx) const
{
    ModelIndexInfo info = m_data->get(idx);
    info.setRect(QRect());
    m_data->update(info);

    //reset rect for children
    invalidateChildrenRect(idx);
}


void PositionsReseter::resetOverallRect(const QModelIndex& idx) const
{
    ModelIndexInfo info = m_data->get(idx);
    info.setOverallRect(QRect());
    m_data->update(info);
}
