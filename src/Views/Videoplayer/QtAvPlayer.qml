import QtQuick 2.12
import QtAV 1.7

Item {
    id: root
    property alias muted: videoPlayer.muted
    property alias position: videoPlayer.position
    property alias duration: videoPlayer.duration
    property alias playbackState: videoPlayer.playbackState
    property alias status: videoPlayer.status
    property alias bufferProgress: videoPlayer.bufferProgress
    property alias source: videoPlayer.source
    property alias playbackRate: videoPlayer.playbackRate
    property alias fillMode: videoOutput.fillMode
    property alias videoPlayerSource: videoPlayer
    property alias videoOutputSource: videoOutput
    property int volume: 0
    property bool isPlaying: false
    property bool isPaused: false
    property bool isStopped: false
    property bool isCropped: false

    signal play();
    signal pause();
    signal stop();
    signal cropModeOutput();
    signal defaultModeOutput();
    signal seek(real position);
    signal playerVolumeChanged(int volume);
    signal playerPlaybackStateChanged(string mode);
    signal playerStatusChanged(string status);
    signal playerPositionChanged(bool isBuffered, int position, int duration);
    signal playerBufferProgressChanged();
    signal playerDurationChanged();

    onVolumeChanged: {
        videoPlayer.volume = root.volume / 100;
        playerVolumeChanged(root.volume);
    }

    onCropModeOutput: {
        videoOutput.fillMode = VideoOutput.PreserveAspectCrop;
        root.isCropped = true;
    }

    onDefaultModeOutput: {
        videoOutput.fillMode = VideoOutput.PreserveAspectFit;
        root.isCropped = false;
    }

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

    VideoOutput2 {
        id: videoOutput
        anchors.fill: parent
        source: videoPlayer
    }

    MediaPlayer {
        id: videoPlayer
        autoPlay: true
        bufferSize: 400
        fastSeek: true
        timeout: 60000
        onBufferProgressChanged: {
            playerBufferProgressChanged();
        }
        onPlaybackStateChanged: {
            root.isPlaying = false;
            root.isPaused = false;
            root.isStopped = false;

            let currentMode = "idle";
            if (videoPlayer.playbackState === MediaPlayer.PlayingState) {
                currentMode = "play";
                root.isPlaying = true;
            }
            if (videoPlayer.playbackState === MediaPlayer.PausedState) {
                currentMode = "pause";
                root.isPaused = true;
            }
            if (videoPlayer.playbackState === MediaPlayer.StoppedState) {
                currentMode = "stop";
                root.isStopped = true;
            }

            playerPlaybackStateChanged(currentMode);
        }
        onStatusChanged: {
            let value = "nostatus";

            if (videoPlayer.status === MediaPlayer.Loading) value = "loading";
            if (videoPlayer.status === MediaPlayer.InvalidMedia) value = "invalid";
            if (videoPlayer.status === MediaPlayer.EndOfMedia) value = "endofmedia";
            if (videoPlayer.status === MediaPlayer.Buffering) value = "buffering";
            if (videoPlayer.status === MediaPlayer.Buffered) value = "buffered";

            playerStatusChanged(value);
        }
        onPositionChanged: {
            playerPositionChanged(videoPlayer.status === MediaPlayer.Buffered, videoPlayer.position, videoPlayer.duration);
        }
        onDurationChanged: {
            playerDurationChanged();
        }
    }
}

