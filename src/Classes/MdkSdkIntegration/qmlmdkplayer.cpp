#include <QDebug>
#include "qmlmdkplayer.h"
#include "videorendererinternal.h"

QmlMDKPlayer::QmlMDKPlayer(QQuickItem *parent):
    QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);

    m_volume = m_player->volume() * 100;
    m_player->setRenderCallback([=](void *){
        QMetaObject::invokeMethod(this, "update");
    });
    m_player->setBufferRange(1000, 3000, false);
    m_player->onStateChanged([=](State state){
        auto stopped = state == State::Stopped || state == State::NotRunning;
        auto paused = state == State::Paused;
        if (stopped || paused) QMetaObject::invokeMethod(this, "stopTimer");
        if (!stopped && !paused) QMetaObject::invokeMethod(this, "startTimer");
        emit isPlayedChanged();
        emit isPausedChanged();
        emit playerStateChanged();
    });
    m_player->onMediaStatusChanged(
        [=](MediaStatus status){
            if (status == MediaStatus::End || status == 65) emit mediaEnded();
            if (status == MediaStatus::Loaded) {
                emit positionChanged();
                emit durationChanged();
            }
            emit bufferingChanged();
            emit mediaStatusChanged();
            return true;
        }
    );
    m_timer->setInterval(500);
    connect(m_timer, &QTimer::timeout, this, &QmlMDKPlayer::playingTimeout);
    m_innerRenderer = new VideoRendererInternal(const_cast<QmlMDKPlayer*>(this));
}

QmlMDKPlayer::~QmlMDKPlayer()
{
    if (m_timer->isActive()) m_timer->stop();
    delete m_player;
}

QQuickFramebufferObject::Renderer *QmlMDKPlayer::createRenderer() const
{
    return m_innerRenderer;
}

void QmlMDKPlayer::setSource(const QString &source) noexcept
{
    if (m_source == source) return;

    m_player->setMedia(source.toUtf8().data());
    m_source = source;
    m_buffering = false;
    emit sourceChanged();
    emit bufferingChanged();
}

void QmlMDKPlayer::setVolume(int volume) noexcept
{
    if (m_volume == volume) return;

    if (volume > 100) volume = 100;
    if (volume < 0) volume = 0;

    m_volume = volume;
    emit volumeChanged();

    float playerVolume = volume;

    m_player->setVolume(playerVolume / 100);
}

void QmlMDKPlayer::setScaleMode(const QmlVideoPlayer::ScaleMode scaleMode) noexcept
{
    if (m_scaleMode == scaleMode) return;

    m_scaleMode = scaleMode;
    emit scaleModeChanged();

    switch (scaleMode) {
        case QmlVideoPlayer::ScaleMode::AspectRatio: {
            m_player->setAspectRatio(KeepAspectRatio);
            break;
        }
        case QmlVideoPlayer::ScaleMode::Crop: {
            m_player->setAspectRatio(KeepAspectRatioCrop);
            break;
        }
        case QmlVideoPlayer::ScaleMode::Stretch: {
            m_player->setAspectRatio(IgnoreAspectRatio);
            break;
        }
    }
}

void QmlMDKPlayer::setIsMute(bool isMute) noexcept
{
    if (m_player->isMute() == isMute) return;

    m_player->setMute(isMute);
    emit isMuteChanged();
}

void QmlMDKPlayer::setPlaybackRate(float rate) noexcept
{
    m_player->setPlaybackRate(rate);

    emit playbackRateChanged();
}

void QmlMDKPlayer::setFreezePosition(bool freezePosition) noexcept
{
    if (m_freezePosition == freezePosition) return;

    m_freezePosition = freezePosition;
    emit freezePositionChanged();
}

void QmlMDKPlayer::setVideoSurfaceSize(int width, int height)
{
    m_player->setVideoSurfaceSize(width, height);
}

void QmlMDKPlayer::renderVideo()
{
    m_player->renderVideo();
}

void QmlMDKPlayer::play() noexcept
{  
    m_player->set(PlaybackState::Playing);
}

void QmlMDKPlayer::pause() noexcept
{
    m_player->set(PlaybackState::Paused);
}

void QmlMDKPlayer::seek(unsigned int position) noexcept
{
    m_player->seek(position);
}

void QmlMDKPlayer::playingTimeout() noexcept
{
    m_buffering = m_player->buffered() == 0;
    if (!m_freezePosition) emit positionChanged();
    emit bufferingChanged();
}

void QmlMDKPlayer::startTimer() noexcept
{
    m_timer->start();
}

void QmlMDKPlayer::stopTimer() noexcept
{
    if (m_timer->isActive()) m_timer->stop();
}
