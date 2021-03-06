/*
 * Photo Broom - photos management tool.
 * Copyright (C) 2015  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "editor_factory.hpp"

#include <cassert>

#include <QCompleter>
#include <QDateEdit>
#include <QHeaderView>
#include <QLineEdit>
#include <QTableWidget>
#include <QTimeEdit>
#include <QTimer>
#include <QDoubleSpinBox>

#include <kratingwidget.h>
#include <kcolorcombo.h>

#include <core/base_tags.hpp>
#include <core/down_cast.hpp>
#include <core/imodel_compositor_data_source.hpp>

#include "utils/model_index_utils.hpp"
#include "utils/svg_utils.hpp"
#include "widgets/tag_editor/helpers/tags_model.hpp"
#include "icompleter_factory.hpp"


namespace
{
    struct TimeEditor: QTimeEdit
    {
        explicit TimeEditor(QWidget* parent_widget = nullptr): QTimeEdit(parent_widget)
        {
            setDisplayFormat("hh:mm:ss");
        }
    };

    template<typename T>
    T* make_editor(ICompleterFactory* completerFactory, const TagTypes& tagType, QWidget* parent)
    {
        T* editor = new T(parent);
        QCompleter* completer = completerFactory->createCompleter(tagType);
        completer->setParent(editor);
        editor->setCompleter(completer);

        return editor;
    }
}


EditorFactory::EditorFactory()
    : m_star()
    , m_completerFactory(nullptr)
{
    QImage star = SVGUtils::load(":/gui/star.svg", {32, 32});
    m_star = QPixmap::fromImage(star);
}


EditorFactory::~EditorFactory()
{

}


void EditorFactory::set(ICompleterFactory* completerFactory)
{
    m_completerFactory = completerFactory;
}


QWidget* EditorFactory::createEditor(const QModelIndex& index, QWidget* parent)
{
    const QVariant tagInfoRoleRaw = index.data(TagsModel::TagInfoRole);
    const TagTypeInfo tagInfoRole = tagInfoRoleRaw.value<TagTypeInfo>();

    return createEditor(tagInfoRole, parent);
}


QWidget* EditorFactory::createEditor(const TagTypeInfo& info, QWidget* parent)
{
    QWidget* result = nullptr;

    const auto tagType = info.getTag();

    switch(tagType)
    {
        case TagTypes::Event:
        case TagTypes::Place:
            result = make_editor<QLineEdit>(m_completerFactory, tagType, parent);
            break;

        case TagTypes::Date:
            result = new QDateEdit(parent);
            break;

        case TagTypes::Time:
            result = new TimeEditor(parent);
            break;

        case TagTypes::Rating:
        {
            KRatingWidget* ratingWidget = new KRatingWidget(parent);
            ratingWidget->setCustomPixmap(m_star);

            result = ratingWidget;
            break;
        }

        case TagTypes::Category:
        {
            KColorCombo* combo = new KColorCombo(parent);
            IModelCompositorDataSource* model = m_completerFactory->accessModel(TagTypes::Category);
            const QStringList& colorsList = model->data();

            QList<QColor> colors;
            for(const QString& colorStr: colorsList)
            {
                const QRgba64 rgba64 = QRgba64::fromRgba64(colorStr.toULongLong());
                const QColor color(rgba64);
                colors.push_back(color);
            }

            combo->setColors(colors);

            result = combo;
            break;
        }

        case TagTypes::Invalid:
        default:
            assert(!"Unexpected call");
            break;
    }

    return result;
}


QByteArray EditorFactory::valuePropertyName(const TagTypeInfo& info) const
{
    QByteArray result;

    const auto tagType = info.getTag();

    switch(tagType)
    {
        case TagTypes::Event:
        case TagTypes::Place:
            result = "text";
            break;

        case TagTypes::Date:
            result = "date";
            break;

        case TagTypes::Time:
            result = "time";
            break;

        case TagTypes::Rating:
            result = "rating";
            break;

        case TagTypes::Category:
            result = "color";
            break;

        case TagTypes::Invalid:
        default:
            assert(!"Unexpected call");
            break;
    }

    return result;
}
