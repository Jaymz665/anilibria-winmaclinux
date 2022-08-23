/*
    AniLibria - desktop client for the website anilibria.tv
    Copyright (C) 2021 Roman Vladimirov

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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia
import "../Controls"
import "../Theme"
import MDKPlayer

Dialog {
    id: root
    x: 10
    y: 10
    width: 350
    height: 200
    leftPadding: 1
    rightPadding: 1
    bottomPadding: 1
    topPadding: 1

    signal showWindow()
    signal hideWindow(bool paused)

    QmlCopyPlayer {
        id: copyPlayer
        regionWidth: dialogContent.width - root.leftPadding
        regionHeight: dialogContent.height
        playerWidth: videoplayer.innerPlayer.width
        playerHeight: videoplayer.innerPlayer.height
    }

    header: Item {
        id: header
        height: 20
        width: parent.width

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 4
            text: "Превью"
        }

        MouseArea {
            anchors.fill: parent

            property real startX: 0
            property real startY: 0
            property real startPositionX: 0
            property real startPositionY: 0

            onPressed: {
                var pos = mapToItem(window.contentItem, mouseX, mouseY)
                startX = pos.x;
                startY = pos.y;

                startPositionX = root.x;
                startPositionY = root.y;
            }

            onPositionChanged: {
                if (!pressed) return;

                var pos = mapToItem(window.contentItem, mouseX, mouseY)
                var deltaX = pos.x-startX;
                var deltaY = pos.y-startY;
                root.x = startPositionX + deltaX;
                root.y = startPositionY + deltaY;
            }
        }

        Item {
            anchors.right: parent.right
            width: 20
            height: header.height

            Image {
                width: 16
                height: 16
                anchors.centerIn: parent
                source: assetsLocation.iconsPath + "coloredclosewindow.svg"
                mipmap: true
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    root.close();
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    MouseArea {
        id: windowedPlayerMouseArea
        width: parent.width
        height: parent.height
        hoverEnabled: true
        cursorShape: onlinePlayerWindowViewModel.windowCursorShape
        onDoubleClicked: {
            mainViewModel.selectPage("videoplayer");
            hideWindow(false);
        }
        onPositionChanged: function (mouse) {
            if (!videoplayer.innerPlayer.isPlayed) {
                if (controlPanel.opacity < 0.1) onlinePlayerWindowViewModel.showPanel();
                return;
            }

            if (controlPanel.opacity < 1) onlinePlayerWindowViewModel.showPanel();
            const x = mouse.x;
            const y = mouse.y;

            if (y > root.height - controlPanel.height) {
                windowPlayerTimer.stop();
            } else {
                windowPlayerTimer.restart();
            }
        }
        onExited: {
            if (!windowPlayerTimer.running) windowPlayerTimer.restart();
        }
    }

    Item {
        id: dialogContent
        anchors.fill: parent

        ShaderEffectSource {
            width: copyPlayer.scaledWidth
            height: copyPlayer.scaledHeight
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            sourceItem: videoplayer.innerPlayer
        }
    }

    Rectangle {
        id: controlPanel
        color: ApplicationTheme.playerControlBackground
        anchors.bottom: parent.bottom
        opacity: onlinePlayerWindowViewModel.panelOpacity
        width: root.width
        height: 60

        RowLayout {
            width: parent.width
            height: parent.height

            Rectangle {
                id: leftPart
                color: "transparent"
                Layout.preferredWidth: 60
                Layout.fillHeight: true

                Slider {
                    id: volumeSlider
                    anchors.centerIn: parent
                    width: parent.width
                    height: 40
                    from: 0
                    value: 10
                    to: 100
                    onPressedChanged: {
                        controlPanel.forceActiveFocus();
                    }
                    onMoved: {
                        videoplayer.innerPlayer.volume = volumeSlider.value;
                        onlinePlayerViewModel.volumeSlider = volumeSlider.value;
                    }
                }
            }

            Item {
                id: centerPart
                Layout.fillWidth: true
                Layout.fillHeight: true

                Row {
                    anchors.centerIn: parent

                    IconButton {
                        id: previousButton
                        width: 40
                        height: 40
                        iconColor: ApplicationTheme.filterIconButtonColor
                        hoverColor: ApplicationTheme.filterIconButtonHoverColor
                        iconPath: assetsLocation.path + "Icons/step-backward.svg"
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            onlinePlayerViewModel.previousVideo();
                        }
                        tooltipMessage: "Предыдущая серия"
                    }

                    IconButton {
                        id: playButton
                        visible: !videoplayer.innerPlayer.isPlayed
                        width: 40
                        height: 40
                        iconColor: ApplicationTheme.filterIconButtonColor
                        hoverColor: ApplicationTheme.filterIconButtonHoverColor
                        iconPath: assetsLocation.path + "Icons/play-button.svg"
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            videoplayer.innerPlayer.play();
                        }
                    }
                    IconButton {
                        id: pauseButton
                        visible: videoplayer.innerPlayer.isPlayed
                        width: 40
                        height: 40
                        iconColor: ApplicationTheme.filterIconButtonColor
                        hoverColor: ApplicationTheme.filterIconButtonHoverColor
                        iconPath: assetsLocation.path + "Icons/pause.svg"
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            videoplayer.innerPlayer.pause();
                        }
                    }

                    IconButton {
                        id: nextButton
                        width: 40
                        height: 40
                        iconColor: ApplicationTheme.filterIconButtonColor
                        hoverColor: ApplicationTheme.filterIconButtonHoverColor
                        iconPath: assetsLocation.path + "Icons/step-forward.svg"
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            onlinePlayerViewModel.nextVideo();
                        }
                        tooltipMessage: "Следующая серия"
                    }
                }
            }

            Rectangle {
                id: rightPart
                color: "transparent"
                Layout.preferredWidth: 45
                Layout.fillHeight: true

                IconButton {
                    id: topmostButton
                    anchors.centerIn: parent
                    width: 40
                    height: 40
                    iconColor: onlinePlayerWindowViewModel.isTopMost ? ApplicationTheme.filterIconButtonGreenColor : ApplicationTheme.filterIconButtonColor
                    hoverColor: ApplicationTheme.filterIconButtonHoverColor
                    iconPath: assetsLocation.path + "Icons/topmostwindow.svg"
                    iconWidth: 29
                    iconHeight: 29
                    tooltipMessage: onlinePlayerWindowViewModel.isTopMost ? "Выключить режим поверх всех окон" : "Включить режим поверх всех окон"
                    onButtonPressed: {
                        onlinePlayerWindowViewModel.isTopMost = !onlinePlayerWindowViewModel.isTopMost;
                        root.flags = onlinePlayerWindowViewModel.isTopMost ? Qt.Dialog | Qt.WindowStaysOnTopHint : Qt.Dialog;
                    }
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    Rectangle {
        width: 80
        height: 80
        color: "white"
        radius: 20
        opacity: 0.8
        visible: videoplayer.innerPlayer.buffering
        anchors.centerIn: parent
        AnimatedImage {
            id: spinner
            anchors.centerIn: parent
            paused: !onlinePlayerWindowViewModel.opened
            playing: onlinePlayerWindowViewModel.opened
            source: assetsLocation.path + "Icons/spinner.gif"
        }
    }

    Item {
        height: 20
        width: 20
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Image {
            anchors.fill: parent
            source: assetsLocation.iconsPath + 'banner.svg'
            mipmap: true
            rotation: 90
        }

        MouseArea {
            anchors.fill: parent
            property real startX: 0
            property real startY: 0
            property real startWidth: 0
            property real startHeight: 0

            onPressed: {
                var pos = mapToItem(window.contentItem, mouseX, mouseY)
                startX = pos.x;
                startY = pos.y;

                startWidth = root.width;
                startHeight = root.height;
            }

            onPositionChanged: {
                if (!pressed) return;

                var pos = mapToItem(window.contentItem, mouseX, mouseY)
                var deltaX = pos.x - startX;
                var deltaY = pos.y - startY;
                const newWidth = startWidth + deltaX;
                const newHeight = startHeight + deltaX;
                root.width = newWidth > 400 ? 400 : newWidth;
                root.height = newHeight > 350 ? 350 : newHeight;
            }
        }
    }

    Timer {
        id: windowPlayerTimer
        interval: 5000
        running: false
        repeat: true
        onTriggered: {
            onlinePlayerWindowViewModel.hideControlPanel()
        }
    }

    onShowWindow:  {
        root.open();
        onlinePlayerWindowViewModel.opened = true;
        onlinePlayerWindowViewModel.showPanel();
    }

    onHideWindow: function (paused) {
        close();
        onlinePlayerWindowViewModel.opened = false;
        if (paused) videoplayer.innerPlayer.pause();
    }

    onClosed: {
        hideWindow(true);
    }
}
