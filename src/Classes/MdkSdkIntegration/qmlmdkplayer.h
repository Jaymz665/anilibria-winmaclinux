#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QQuickFramebufferObject>
#include <QDebug>
#include <QTimer>
#include "mdk/Player.h"

using namespace MDK_NS;

namespace QmlVideoPlayer
{
    Q_NAMESPACE         // required for meta object creation
    enum ScaleMode {
        Stretch = 1,
        AspectRatio = 2,
        Crop = 3
    };
    Q_ENUM_NS(ScaleMode)  // register the enum in meta object data
}

class QmlMDKPlayer : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QmlVideoPlayer::ScaleMode scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(bool isPlayed READ isPlayed NOTIFY isPlayedChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY isPausedChanged)
    Q_PROPERTY(int position READ position NOTIFY positionChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(bool freezePosition READ freezePosition WRITE setFreezePosition NOTIFY freezePositionChanged)
    Q_PROPERTY(bool buffering READ buffering NOTIFY bufferingChanged)
    Q_PROPERTY(bool isMute READ isMute WRITE setIsMute NOTIFY isMuteChanged)
    Q_PROPERTY(float playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)

private:
    QString m_source;
    uint32_t m_volume;
    QmlVideoPlayer::ScaleMode m_scaleMode;
    Player* m_player { new Player() };
    QTimer* m_timer { new QTimer(this) };
    bool m_freezePosition { false };
    bool m_buffering { false };
    QQuickFramebufferObject::Renderer * m_innerRenderer { nullptr };

public:
    explicit QmlMDKPlayer(QQuickItem *parent = nullptr);
    virtual ~QmlMDKPlayer();

    Renderer *createRenderer() const;

    QString source() const noexcept { return m_source; }
    void setSource(const QString & source) noexcept;

    int volume() const noexcept { return m_volume; }
    void setVolume(int volume) noexcept;

    QmlVideoPlayer::ScaleMode scaleMode() const noexcept { return m_scaleMode; }
    void setScaleMode(const QmlVideoPlayer::ScaleMode scaleMode) noexcept;

    bool isMute() const noexcept { return m_player->isMute(); }
    void setIsMute(bool isMute) noexcept;

    float playbackRate() const noexcept { return m_player->playbackRate(); }
    void setPlaybackRate(float playbackRate) noexcept;

    bool freezePosition() const noexcept { return m_freezePosition; }
    void setFreezePosition(bool freezePosition) noexcept;

    bool isPlayed() const noexcept { return m_player->state() == PlaybackState::Playing; }
    bool isPaused() const noexcept { return m_player->state() == PlaybackState::Paused; }
    int position() const noexcept { return m_player->position(); }
    int duration() const noexcept { return m_player->mediaInfo().duration; }
    bool buffering() const noexcept { return m_player->mediaStatus() == MediaStatus::Loading || m_buffering; }

    void setVideoSurfaceSize(int width, int height);
    void renderVideo();

    Q_INVOKABLE void play() noexcept;
    Q_INVOKABLE void pause() noexcept;
    Q_INVOKABLE void stop() noexcept;
    Q_INVOKABLE void seek(unsigned int position) noexcept;

private slots:
    void playingTimeout() noexcept;
    void startTimer() noexcept;
    void stopTimer() noexcept;

signals:
    void sourceChanged();
    void volumeChanged();
    void scaleModeChanged();
    void isPlayedChanged();
    void isPausedChanged();
    void mediaEnded();
    void positionChanged();
    void durationChanged();
    void freezePositionChanged();
    void bufferingChanged();
    void isMuteChanged();
    void mediaStatusChanged();
    void playerStateChanged();
    void playbackRateChanged();

};

#endif // VIDEORENDERER_H
