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
import QtMultimedia

Item {
    property alias muted: videoPlayer.audioOutput.muted
    property alias volume: videoPlayer.audioOutput.volume
    property alias position: videoPlayer.position
    property alias duration: videoPlayer.duration
    property alias playbackState: videoPlayer.playbackState
    property int status: videoPlayer.mediaStatus
    property alias bufferProgress: videoPlayer.bufferProgress
    property alias source: videoPlayer.source
    property alias playbackRate: videoPlayer.playbackRate
    property alias fillMode: videoOutput.fillMode
    property alias videoPlayerSource: videoPlayer
    property alias videoOutputSource: videoOutput

    signal play();
    signal pause();
    signal stop();
    signal seek(real position);
    signal playerVolumeChanged();
    signal playerPlaybackStateChanged();
    signal playerStatusChanged();
    signal playerPositionChanged();
    signal playerBufferProgressChanged();
    signal playerDurationChanged();

    onPlay: {
        videoPlayer.play();
    }

    onPause: {
        videoPlayer.pause();
    }

    onStop: {
        videoPlayer.stop();
    }

    onSeek: {
        videoPlayer.seek(position);
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        onStateChanged: {
        }
    }

    MediaPlayer {
        id: videoPlayer
        videoOutput: videoOutput
        audioOutput: AudioOutput {
            onVolumeChanged: {
                playerVolumeChanged();
            }
        }

        signal addNewVideoOuput(var newVideoOutput);

        onAddNewVideoOuput: function (newVideoOutput) {
            console.log(newVideoOutput);
            //videoPlayer.videoOutput.push(newVideoOutput);
        }
        onBufferProgressChanged: {
            playerBufferProgressChanged();
        }
        onPlaybackStateChanged: {
            playerPlaybackStateChanged();
        }
        onMediaStatusChanged: {
            playerStatusChanged();
        }
        onPositionChanged: {
            playerPositionChanged();
        }
        onDurationChanged: {
            playerDurationChanged();
        }
    }
}

