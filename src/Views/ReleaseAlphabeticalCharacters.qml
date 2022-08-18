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

import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import "../Controls"
import "../Theme"

Item {
    anchors.fill: parent

    MouseArea {
        anchors.fill: parent
        onPressed: {
            page.showAlpabeticalCharaters = false;
        }
    }

    Rectangle {
        color: "black"
        anchors.fill: parent
        opacity: .5
    }

    GridView {
        width: 800
        height: 300
        cellWidth: 80
        cellHeight: 80
        anchors.centerIn: parent
        model: alphabetListModel
        delegate: Rectangle {
            width: 78
            height: 78

            ToggleButton {
                anchors.fill: parent
                text: alphabetCharacter
                isChecked: characterSelected
                onButtonClicked: {
                    alphabetListModel.selectCharacter(alphabetCharacter)

                    releasesViewModel.items.alphabetsFilter = alphabetListModel.getSelectedCharactersAsString();

                    releasesViewModel.items.refresh();
                }
            }
        }
    }
}
