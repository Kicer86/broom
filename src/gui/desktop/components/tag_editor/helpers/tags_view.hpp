/*
 * View for tags
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

#ifndef TAGSVIEW_HPP
#define TAGSVIEW_HPP

#include <QTableView>

#include "components/editor_factory.hpp"

class TagsView: public QTableView
{
    public:
        TagsView(QWidget * = 0);
        TagsView(const TagsView &) = delete;
        ~TagsView();

        TagsView& operator=(const TagsView &) = delete;

    private:
        EditorFactory m_editorFactory;

        bool edit(const QModelIndex &, EditTrigger, QEvent *) override;
};

#endif // TAGSVIEW_HPP
