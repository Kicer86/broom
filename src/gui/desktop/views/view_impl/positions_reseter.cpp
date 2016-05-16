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

#ifndef NDEBUG
    #include <iostream>
#endif

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QModelIndex>

#include "data.hpp"

PositionsReseter::PositionsReseter(QAbstractItemModel* model, Data* data): m_data(data), m_model(model)
{

}


PositionsReseter::~PositionsReseter()
{

}


void PositionsReseter::itemsAdded(const QModelIndex& parent, int /*from_pos*/, int to_pos) const
{
    //invalidate parent
    invalidateItemOverallSize(parent);

    //invalidate all items which are after 'to_pos'
    const QModelIndex item = m_model->index(to_pos, 0, parent);
    invalidateSiblingsPosition(item);
}


void PositionsReseter::invalidateAll() const
{
    Data::ModelIndexInfoSet& dataSet = m_data->getModel();
    for(auto it = dataSet.begin(); it != dataSet.end(); ++it)
    {
        ModelIndexInfo& info = it->second;

        info.cleanRects();
    };
}


void PositionsReseter::itemChanged(const QModelIndex& idx)
{
    //invalidate parent's overallRect + positions of parent's siblings
    const QModelIndex parent = idx.parent();
    resetOverallSize(parent);
    invalidateSiblingsPosition(parent);

    //invalidate itself
    resetSize(idx);
    resetOverallSize(idx);

    //invalidate all items which are after 'pos'
    invalidateSiblingsPosition(idx);
}


void PositionsReseter::itemsChanged(const QItemSelection& selectedItems)
{
    const QModelIndexList items = selectedItems.indexes();
    const int s = items.count();

    if (s > 0)
    {
        const QModelIndex& first = items[0];

        //invalidate parent
        const QModelIndex parent = first.parent();
        invalidateItemOverallSize(parent);

        //invalidate themselves
        for(const QModelIndex& item: items)
            resetRect(item);

        const QModelIndex& last = items[s - 1];

        //invalidate all items which are after 'pos'
        invalidateSiblingsPosition(last);
    }
}


void PositionsReseter::childrenRemoved(const QModelIndex& parent, int pos)
{
    //invalidate parent if expanded
    Data::ModelIndexInfoSet::iterator infoIt = m_data->find(parent);

    if (m_data->isValid(infoIt))
    {
        const ModelIndexInfo& parentInfo = infoIt->second;

        if (parentInfo.expanded)
            invalidateItemOverallSize(parent);

        //invalidate all items which are after 'pos'
        for(QModelIndex idx = parent.child(pos, 0); idx.isValid(); idx = parent.child(++pos, 0))
            resetPosition(idx);
    }
}


void PositionsReseter::invalidateItemOverallSize(const QModelIndex& idx) const
{
    resetOverallSize(idx);
    invalidateSiblingsPosition(idx);

    if (idx != QModelIndex())                           //do not invalidate root's parent if it doesn't exist
    {
        //if 'this' becomes invalid, invalidate also its parent
        const QModelIndex parent = idx.parent();
        invalidateItemOverallSize(parent);
    }
}


void PositionsReseter::invalidateSiblingsRect(const QModelIndex& idx) const
{
    if (idx.isValid())
    {
        int row = idx.row() + 1;

        for(QModelIndex sibling = idx.sibling(row, 0); sibling.isValid(); sibling = sibling.sibling(++row, 0))
        {
            // if 'sibling' is invalid, all after it are invalid also
            Data::ModelIndexInfoSet::iterator siblingIt = m_data->find(sibling);
            if ( m_data->isValid(siblingIt) == false)
                break;

            resetRect(sibling);

            //reset rect for children
            invalidateChildrenRect(sibling);
        }
    }
}


void PositionsReseter::invalidateSiblingsPosition(const QModelIndex& idx) const
{
    if (idx.isValid())
    {
        int row = idx.row() + 1;

        for(QModelIndex sibling = idx.sibling(row, 0); sibling.isValid(); sibling = sibling.sibling(++row, 0))
        {
            // if 'sibling' is invalid, all after it are invalid also
            Data::ModelIndexInfoSet::iterator siblingIt = m_data->find(sibling);
            if ( m_data->isValid(siblingIt) == false)
                break;

            resetPosition(sibling);
        }
    }
}


void PositionsReseter::invalidateChildrenRect(const QModelIndex& parent, int from) const
{
    int r = from;
    for(QModelIndex child = m_model->index(r, 0, parent); child.isValid(); child = m_model->index(++r, 0, parent))
    {
        resetRect(child);

        //reset rect for children
        invalidateChildrenRect(child);
    }
}


void PositionsReseter::resetRect(const QModelIndex& idx) const
{
    Data::ModelIndexInfoSet::iterator infoIt = m_data->find(idx);

    if ( m_data->isValid(infoIt))
    {
        ModelIndexInfo& info = infoIt->second;
        info.markRectInvalid();
    }
}


void PositionsReseter::resetPosition(const QModelIndex& idx) const
{
    Data::ModelIndexInfoSet::iterator infoIt = m_data->find(idx);

    if ( m_data->isValid(infoIt))
    {
        ModelIndexInfo& info = infoIt->second;
        info.markPositionInvalid();
    }
}


void PositionsReseter::resetSize(const QModelIndex& idx) const
{
    Data::ModelIndexInfoSet::iterator infoIt = m_data->find(idx);

    if ( m_data->isValid(infoIt))
    {
        ModelIndexInfo& info = infoIt->second;
        info.markSizeInvalid();
    }
}


void PositionsReseter::resetOverallSize(const QModelIndex& idx) const
{
    Data::ModelIndexInfoSet::iterator infoIt = m_data->find(idx);

    if ( m_data->isValid(infoIt))
    {
        ModelIndexInfo& info = infoIt->second;
        info.markOverallSizeInvalid();
    }
}


void PositionsReseter::resetPosition(Data::ModelIndexInfoSet::iterator it) const
{
    it->second.markPositionInvalid();
}


void PositionsReseter::resetSize(Data::ModelIndexInfoSet::iterator it) const
{
    it->second.markSizeInvalid();
}
