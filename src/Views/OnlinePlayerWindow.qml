import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "../Controls"
import "Videoplayer"

ApplicationWindow {
    id: root
    title: '  '
    width: 350
    height: 200
    flags: Qt.Dialog
    minimumWidth: 350
    minimumHeight: 200
    maximumWidth: 500
    maximumHeight: 350

    property var videoSource
    property var videoOutput
    property var videoOrigin
    property bool needLoadPlayer

    signal showWindow()
    signal hideWindow(bool paused)
    signal closeWindow()
    signal loadPlayer()

    function playerLoadedHandler() {
        if (!onlinePlayerWindowViewModel.supportOutput) videoOutputLoader.item.source = root.videoSource;
        if (onlinePlayerWindowViewModel.supportOutput) {
            root.videoSource.addNewVideoOuput(videoOutputLoader.item);
        }
        root.videoOrigin.playerPlaybackStateChanged.connect(playbackStateChanged);
        root.videoOrigin.volumeChanged.connect(volumeChanged);
        volumeSlider.value = root.videoOrigin.volume;
    }

    Loader {
        id: videoOutputLoader
        anchors.fill: parent
        source: onlinePlayerWindowViewModel.playerOutputComponent
        onLoaded: {
            if (needLoadPlayer) playerLoadedHandler();
        }
    }

    onLoadPlayer: {
        if (videoOutputLoader.status == Loader.Ready) {
            needLoadPlayer = false;
            playerLoadedHandler();
        } else {
            needLoadPlayer = true;
        }
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
        onPositionChanged: {
            if (!root.videoOrigin.isPlaying) {
                if (controlPanel.opacity === 0) onlinePlayerWindowViewModel.showPanel();
                return;
            }

            if (controlPanel.opacity !== 1) onlinePlayerWindowViewModel.showPanel();
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

    Rectangle {
        id: controlPanel
        color: applicationThemeViewModel.playerControlBackground
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
                        root.videoOrigin.volume = value;
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
                        hoverColor: applicationThemeViewModel.filterIconButtonHoverColor
                        iconPath: applicationThemeViewModel.currentItems.iconPlayerPreviousSeria
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            onlinePlayerViewModel.previousVideo();
                        }
                        tooltipMessage: "Предыдущая серия"
                    }

                    IconButton {
                        id: playButton
                        visible: onlinePlayerWindowViewModel.playerButtonVisible
                        width: 40
                        height: 40
                        hoverColor: applicationThemeViewModel.filterIconButtonHoverColor
                        iconPath: applicationThemeViewModel.currentItems.iconPlayerPlay
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            root.videoOrigin.play();
                        }
                    }
                    IconButton {
                        id: pauseButton
                        visible: onlinePlayerWindowViewModel.pauseButtonVisible
                        width: 40
                        height: 40
                        hoverColor: applicationThemeViewModel.filterIconButtonHoverColor
                        iconPath: applicationThemeViewModel.currentItems.iconPlayerPause
                        iconWidth: 24
                        iconHeight: 24
                        onButtonPressed: {
                            root.videoOrigin.pause();
                        }
                    }

                    IconButton {
                        id: nextButton
                        width: 40
                        height: 40
                        hoverColor: applicationThemeViewModel.filterIconButtonHoverColor
                        iconPath: applicationThemeViewModel.currentItems.iconPlayerNextSeria
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
                    hoverColor: applicationThemeViewModel.filterIconButtonHoverColor
                    iconPath: applicationThemeViewModel.currentItems.iconPlayerTopMost
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
        visible: onlinePlayerViewModel.isBuffering
        anchors.centerIn: parent
        AnimatedImage {
            id: spinner
            anchors.centerIn: parent
            paused: !onlinePlayerWindowViewModel.opened
            playing: onlinePlayerWindowViewModel.opened
            source: assetsLocation.path + "Icons/spinner.gif"
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

    function playbackStateChanged(currentMode) {
        onlinePlayerWindowViewModel.playbackStateChanged(currentMode === `play`);
    }

    function volumeChanged() {
        volumeSlider.value = root.videoOrigin.volume;
    }

    onShowWindow:  {
        videoOutputLoader.item.visible = true;
        show();
        onlinePlayerWindowViewModel.opened = true;
        onlinePlayerWindowViewModel.showPanel();
    }

    onHideWindow: {
        videoOutputLoader.item.visible = false;
        hide();
        onlinePlayerWindowViewModel.opened = false;
        if (paused) root.videoOrigin.pause();
    }

    onClosing: {
        hideWindow(true);
    }

    onCloseWindow: {
        root.videoOrigin.playerPlaybackStateChanged.disconnect(playbackStateChanged);
        root.videoOrigin.volumeChanged.disconnect(volumeChanged);
    }
}
