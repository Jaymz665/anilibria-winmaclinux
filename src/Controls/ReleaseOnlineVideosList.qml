import QtQuick 2.12
import QtQuick.Controls 2.12
import Anilibria.ListModels 1.0

Item {
    id: root
    width: parent.width
    height: 330

    property alias releaseId: viewModel.releaseId
    property alias isEmpty: viewModel.isEmpty

    signal openVideo(int videoId)

    ReleaseOnlineSeriesListModel {
        id: viewModel
        releases: releasesViewModel
        onlinePlayer: onlinePlayerViewModel
        onSelectedVideoIndexChanged: {
            if (selectedVideoIndex == -1) return;

            videosListView.positionViewAtIndex(selectedVideoIndex, ListView.Contain);
        }
    }

    Rectangle {
        id: mask
        width: 180
        height: 260
        radius: 8
        visible: false
    }

    ListView {
        id: videosListView
        anchors.fill: parent
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        model: viewModel
        clip: true
        ScrollBar.horizontal: ScrollBar {
            active: true
        }
        delegate: Item {
            width: 410
            height: 330

            Rectangle {
                id: posterImage
                color: "transparent"
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: 8
                width: isCurrentVideo ? 404 : 400
                height: isCurrentVideo ? 274 : 272
                border.color: isCurrentVideo ? applicationThemeViewModel.selectedItem : "transparent"
                border.width: isCurrentVideo ? 2 : 0
                radius: 4

                property bool posterHovered: false

                CorneredImage {
                    anchors.centerIn: parent
                    posterSource: poster
                    preserveAspectCrop: isEmptyPoster ? Image.Stretch : Image.PreserveAspectCrop
                    width: isEmptyPoster ? 100 : 400
                    height: isEmptyPoster ? 100 : 270
                }

                Image {
                    id: bookmarkImage
                    anchors.left: posterImage.left
                    anchors.leftMargin: -7
                    width: 50
                    height: 50
                    source: assetsLocation.iconsPath + "numberbookmark.svg"
                }

                PlainText {
                    anchors.left: bookmarkImage.left
                    anchors.top: bookmarkImage.top
                    anchors.topMargin: 22
                    width: 40
                    horizontalAlignment: Qt.AlignHCenter
                    text: id
                    fontPointSize: 10
                    color: applicationThemeViewModel.colorPageIndexText
                }

                Rectangle {
                    id: blockedContainer
                    visible: posterImage.posterHovered
                    anchors.fill: parent
                    color: "black"
                    opacity: 0.4
                    radius: 12
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        posterImage.posterHovered = true;
                    }
                    onExited: {
                        posterImage.posterHovered = false;
                    }
                    onPressed: {
                        openVideo(index);
                    }
                }
            }
            AccentText {
                id: seenText
                anchors.top: posterImage.bottom
                width: parent.width
                text: isSeens ? "Просмотрено" : currentTimeVideo
                fontPointSize: 10
                maximumLineCount: 1
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                horizontalAlignment: Qt.AlignHCenter
            }

            PlainText {
                anchors.top: seenText.bottom
                width: parent.width
                height: 30
                fontPointSize: 10
                maximumLineCount: 1
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: description
            }
        }
    }

    NavigationScrollButton {
        visible: videosListView.contentX > 100
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.top: parent.top
        anchors.topMargin: 120
        scrollView: videosListView
        isLeft: true
    }

    NavigationScrollButton {
        visible: !videosListView.atXEnd
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.top: parent.top
        anchors.topMargin: 120
        scrollView: videosListView
        isLeft: false
    }

    EmptyBoxArea {
        visible: viewModel.isEmpty
        anchors.fill: parent
        title: "Нет онлайн серий"
    }
}
