#include "videorendererinternal.h"

VideoRendererInternal::VideoRendererInternal(QmlMDKPlayer *player)
{
    m_player = player;
}

void VideoRendererInternal::render()
{
    m_player->renderVideo();
}

QOpenGLFramebufferObject *VideoRendererInternal::createFramebufferObject(const QSize &size)
{
    m_player->setVideoSurfaceSize(size.width(), size.height());
    return new QOpenGLFramebufferObject(size);
}
