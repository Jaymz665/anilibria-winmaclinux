#ifndef QMLCOPYPLAYER_H
#define QMLCOPYPLAYER_H

#include <QObject>

class QmlCopyPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int playerWidth READ playerWidth WRITE setPlayerWidth NOTIFY playerWidthChanged)
    Q_PROPERTY(int playerHeight READ playerHeight WRITE setPlayerHeight NOTIFY playerHeightChanged)
    Q_PROPERTY(int regionWidth READ regionWidth WRITE setRegionWidth NOTIFY regionWidthChanged)
    Q_PROPERTY(int regionHeight READ regionHeight WRITE setRegionHeight NOTIFY regionHeightChanged)
    Q_PROPERTY(int scaledWidth READ scaledWidth WRITE setScaledWidth NOTIFY scaledWidthChanged)
    Q_PROPERTY(int scaledHeight READ scaledHeight WRITE setScaledHeight NOTIFY scaledHeightChanged)

private:
    int m_playerWidth { 0 };
    int m_playerHeight { 0 };
    int m_regionWidth { 0 };
    int m_regionHeight { 0 };
    int m_scaledWidth { 0 };
    int m_scaledHeight { 0 };

public:
    explicit QmlCopyPlayer(QObject *parent = nullptr);

    int playerWidth() const noexcept { return m_playerWidth; }
    int playerHeight() const noexcept { return m_playerHeight; }
    int regionWidth() const noexcept { return m_regionWidth; }
    int regionHeight() const noexcept { return m_regionHeight; }
    int scaledWidth() const noexcept { return m_scaledWidth; }
    int scaledHeight() const noexcept { return m_scaledHeight; }

    void setPlayerWidth(int playerWidth) noexcept;
    void setPlayerHeight(int playerHeight) noexcept;
    void setRegionWidth(int regionWidth) noexcept;
    void setRegionHeight(int regionHeight) noexcept;
    void setScaledWidth(int scaledWidth) noexcept;
    void setScaledHeight(int scaledHeight) noexcept;

private:
    void calculate() noexcept;

signals:
    void playerWidthChanged();
    void playerHeightChanged();
    void regionWidthChanged();
    void regionHeightChanged();
    void scaledWidthChanged();
    void scaledHeightChanged();

};

#endif // QMLCOPYPLAYER_H
