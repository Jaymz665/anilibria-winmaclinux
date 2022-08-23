#ifndef VIDEORENDERERINTERNAL_H
#define VIDEORENDERERINTERNAL_H

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>
#include "qmlmdkplayer.h"

class VideoRendererInternal : public QQuickFramebufferObject::Renderer
{
private:
    QmlMDKPlayer * m_player { nullptr };

public:
    VideoRendererInternal(QmlMDKPlayer *player);

    void render() override;

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
};

#endif // VIDEORENDERERINTERNAL_H
