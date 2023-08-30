/*
    AniLibria - desktop client for the website anilibria.tv
    Copyright (C) 2020 Roman Vladimirov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.12
import QtQuick.Controls 2.12
import CustomStyle 1.0

MenuItem {
    property bool notVisible: false

    CustomStyle.menuForeground: window.colorMenuForeground
    CustomStyle.menuItemHint: window.colorMenuItemHint
    CustomStyle.menuRipple: window.colorMenuRipple
    CustomStyle.menuHighlight: window.colorMenuHighlight

    width: parent.width
    height: notVisible ? 0 : undefined
    visible: !notVisible
    font.pointSize: 11 + windowSettings.dpiSeparation
}
