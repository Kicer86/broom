/*
 * Color pick button
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

#ifndef COLORPICKBUTTON_HPP
#define COLORPICKBUTTON_HPP

#include <QPushButton>

class ColorPickButton: public QPushButton
{
    public:
        ColorPickButton(QWidget * = nullptr);
        ColorPickButton(const ColorPickButton &) = delete;
        ~ColorPickButton();

        ColorPickButton& operator=(const ColorPickButton &) = delete;

        void setColor(const QColor &);

        const QColor& getColor() const;

    private:
        QColor m_color;

        void applyColor();
};

#endif // COLORPICKBUTTON_HPP
