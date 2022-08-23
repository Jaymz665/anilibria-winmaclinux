#include "qmlcopyplayer.h"
#include "videorendererinternal.h"

QmlCopyPlayer::QmlCopyPlayer(QObject *parent) :
    QObject(parent)
{

}

void QmlCopyPlayer::setPlayerWidth(int playerWidth) noexcept
{
    if (m_playerWidth == playerWidth) return;

    m_playerWidth = playerWidth;
    emit playerWidthChanged();

    calculate();
}

void QmlCopyPlayer::setPlayerHeight(int playerHeight) noexcept
{
    if (m_playerHeight == playerHeight) return;

    m_playerHeight = playerHeight;
    emit playerHeightChanged();

    calculate();
}

void QmlCopyPlayer::setRegionWidth(int regionWidth) noexcept
{
    if (m_regionWidth == regionWidth) return;

    m_regionWidth = regionWidth;
    emit regionWidthChanged();

    calculate();
}

void QmlCopyPlayer::setRegionHeight(int regionHeight) noexcept
{
    if (m_regionHeight == regionHeight) return;

    m_regionHeight = regionHeight;
    emit regionHeightChanged();

    calculate();
}

void QmlCopyPlayer::setScaledWidth(int scaledWidth) noexcept
{
    if (m_scaledWidth == scaledWidth) return;

    m_scaledWidth = scaledWidth;
    emit scaledWidthChanged();
}

void QmlCopyPlayer::setScaledHeight(int scaledHeight) noexcept
{
    if (m_scaledHeight == scaledHeight) return;

    m_scaledHeight = scaledHeight;
    emit scaledHeightChanged();
}

void QmlCopyPlayer::calculate() noexcept
{
    if (m_playerWidth == 0 || m_playerHeight == 0) return;

    float ratioWidth = static_cast<float>(m_regionWidth) / static_cast<float>(m_playerWidth);
    float ratioHeight = static_cast<float>(m_regionHeight) / static_cast<float>(m_playerHeight);

    auto ratio = qMin(ratioWidth, ratioHeight);

    setScaledWidth(m_playerWidth * ratio);
    setScaledHeight(m_playerHeight * ratio);
}
