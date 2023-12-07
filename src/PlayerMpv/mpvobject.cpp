#include <stdexcept>
#include <clocale>
#include <QObject>
#include <QtGlobal>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QOpenGLFramebufferObject>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>
#include <QThread>
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#include <QQuickOpenGLUtils>
#endif
#include "mpvobject.h"
#include "qthelper.h"

void on_mpv_events(void *ctx)
{
    Q_UNUSED(ctx);
}

void on_mpv_redraw(void *ctx)
{
    MpvObject::on_update(ctx);
}

static void *get_proc_address_mpv(void *ctx, const char *name)
{
    Q_UNUSED(ctx)

    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;

    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

struct nodeAutoFree {
    mpv_node *ptr;

    nodeAutoFree(mpv_node *a_ptr) : ptr(a_ptr) {}
    ~nodeAutoFree() { mpv_free_node_contents(ptr); }
};

struct ErrorReturn
{
    int error;

    ErrorReturn() : error(0) {}
    explicit ErrorReturn(int err) : error(err) {}
};

struct nodeBuilder {
    nodeBuilder(const QVariant& v) {
        set(&node_, v);
    }
    ~nodeBuilder() {
        free_node(&node_);
    }
    mpv_node *node() { return &node_; }
private:
    Q_DISABLE_COPY(nodeBuilder)
    mpv_node node_;
    mpv_node_list *create_list(mpv_node *dst, bool is_map, int num) {
        dst->format = is_map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;
        mpv_node_list *list = new mpv_node_list();
        dst->u.list = list;
        if (!list)
            goto err;
        list->values = new mpv_node[num]();
        if (!list->values)
            goto err;
        if (is_map) {
            list->keys = new char*[num]();
            if (!list->keys)
                goto err;
        }
        return list;
    err:
        free_node(dst);
        return NULL;
    }
    char *dup_qstring(const QString &s) {
        QByteArray b = s.toUtf8();
        char *r = new char[b.size() + 1];
        if (r)
            std::memcpy(r, b.data(), b.size() + 1);
        return r;
    }
    bool test_type(const QVariant &v, QMetaType::Type t) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        return v.typeId() == static_cast<int>(t);
#else
        return static_cast<int>(v.type()) == static_cast<int>(t);
#endif
    }

    void set(mpv_node *dst, const QVariant &src) {
        if (test_type(src, QMetaType::QString)) {
            dst->format = MPV_FORMAT_STRING;
            dst->u.string = dup_qstring(src.toString());
            if (!dst->u.string)
                goto fail;
        } else if (test_type(src, QMetaType::Bool)) {
            dst->format = MPV_FORMAT_FLAG;
            dst->u.flag = src.toBool() ? 1 : 0;
        } else if (test_type(src, QMetaType::Int) ||
                   test_type(src, QMetaType::LongLong) ||
                   test_type(src, QMetaType::UInt) ||
                   test_type(src, QMetaType::ULongLong))
        {
            dst->format = MPV_FORMAT_INT64;
            dst->u.int64 = src.toLongLong();
        } else if (test_type(src, QMetaType::Double)) {
            dst->format = MPV_FORMAT_DOUBLE;
            dst->u.double_ = src.toDouble();
        } else if (src.canConvert<QVariantList>()) {
            QVariantList qlist = src.toList();
            mpv_node_list *list = create_list(dst, false, qlist.size());
            if (!list)
                goto fail;
            list->num = qlist.size();
            for (int n = 0; n < qlist.size(); n++)
                set(&list->values[n], qlist[n]);
        } else if (src.canConvert<QVariantMap>()) {
            QVariantMap qmap = src.toMap();
            mpv_node_list *list = create_list(dst, true, qmap.size());
            if (!list)
                goto fail;
            list->num = qmap.size();
            for (int n = 0; n < qmap.size(); n++) {
                auto keys = qmap.keys();
                auto key = keys[n];
                list->keys[n] = dup_qstring(key);
                if (!list->keys[n]) {
                    free_node(dst);
                    goto fail;
                }
                auto values = qmap.values();
                auto value = values[n];
                set(&list->values[n], value);
            }
        } else {
            goto fail;
        }
        return;
    fail:
        dst->format = MPV_FORMAT_NONE;
    }
    void free_node(mpv_node *dst) {
        switch (dst->format) {
        case MPV_FORMAT_STRING:
            delete[] dst->u.string;
            break;
        case MPV_FORMAT_NODE_ARRAY:
        case MPV_FORMAT_NODE_MAP: {
            mpv_node_list *list = dst->u.list;
            if (list) {
                for (int n = 0; n < list->num; n++) {
                    if (list->keys)
                        delete[] list->keys[n];
                    if (list->values)
                        free_node(&list->values[n]);
                }
                delete[] list->keys;
                delete[] list->values;
            }
            delete list;
            break;
        }
        default: ;
        }
        dst->format = MPV_FORMAT_NONE;
    }
};

class MpvRenderer : public QQuickFramebufferObject::Renderer
{
    MpvObject *obj;

public:
    MpvRenderer(MpvObject *new_obj)
        : obj{new_obj}
    {
        mpv_set_wakeup_callback(obj->mpv, on_mpv_events, obj);
    }

    virtual ~MpvRenderer()
    {}

    // This function is called when a new FBO is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject * createFramebufferObject(const QSize &size)
    {
        if (!obj->mpv_gl)
        {
            mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr};
            mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                {MPV_RENDER_PARAM_INVALID, nullptr}
            };

            if (mpv_render_context_create(&obj->mpv_gl, obj->mpv, params) < 0) {
                throw std::runtime_error("failed to initialize mpv GL context");
            }
            mpv_render_context_set_update_callback(obj->mpv_gl, on_mpv_redraw, obj);
        }

        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render()
    {
        #if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
            QQuickOpenGLUtils::resetOpenGLState();
        #else
            obj->window()->resetOpenGLState();
        #endif

        QOpenGLFramebufferObject *fbo = framebufferObject();
        mpv_opengl_fbo mpfbo{.fbo = static_cast<int>(fbo->handle()), .w = fbo->width(), .h = fbo->height(), .internal_format = 0};
        int flip_y{0};

        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };
        mpv_render_context_render(obj->mpv_gl, params);

        #if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
            QQuickOpenGLUtils::resetOpenGLState();
        #else
            obj->window()->resetOpenGLState();
        #endif
    }
};

MpvObject::MpvObject(QQuickItem * parent)
    : QQuickFramebufferObject(parent), mpv{mpv_create()}, mpv_gl(nullptr)
{
    if (!mpv) throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "all=v");
    mpv_set_option_string(mpv, "cache", "yes");
    mpv_set_option_string(mpv, "cache-secs", "4");

    if (mpv_initialize(mpv) < 0) throw std::runtime_error("could not initialize mpv context");

    mpv_set_option_string(mpv, "hwdec", "auto");

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "mute", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);

    m_checkTimer = startTimer(80);

    connect(this, &MpvObject::onUpdate, this, &MpvObject::doUpdate, Qt::QueuedConnection);
}

MpvObject::~MpvObject()
{
    if (mpv_gl) mpv_render_context_free(mpv_gl);

    mpv_terminate_destroy(mpv);
}

void MpvObject::on_update(void *ctx)
{
    MpvObject *self = (MpvObject *)ctx;
    emit self->onUpdate();
}

// connected to onUpdate(); signal makes sure it runs on the GUI thread
void MpvObject::doUpdate()
{
    update();
}

void MpvObject::command(const QVariant& params)
{
    mpv::qt::command(mpv, params);
}

void MpvObject::setProperty(const QString& name, const QVariant& value)
{
    setMpvProperty(mpv, name, value);
}

QQuickFramebufferObject::Renderer *MpvObject::createRenderer() const
{
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    window()->setPersistentGraphics(true);
#else
    window()->setPersistentOpenGLContext(true);
#endif

    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvObject *>(this));
}

int MpvObject::volume() const noexcept
{
    auto value = mpv::qt::get_property(mpv, "volume");
    if (value.userType() == QMetaType::Int) value = value.toInt();

    return 100;
}

void MpvObject::setVolume(int volume) noexcept
{
    setMpvProperty(mpv, "volume", volume);

    emit volumeChanged();
}

bool MpvObject::muted() const noexcept
{
    auto value = mpv::qt::get_property(mpv, "mute");
    if (value.userType() == QMetaType::Bool) return value.toBool();

    return false;
}

void MpvObject::setMuted(bool muted) noexcept
{
    setMpvProperty(mpv, "mute", muted);

    emit mutedChanged();
}

void MpvObject::setSource(const QString& source) noexcept
{
    if (source == m_source) return;

    m_source = source;

    QStringList items;
    items.append("loadfile");
    items.append(source);
    mpv::qt::command(mpv, QVariant(items));

    emit sourceChanged();
}

float MpvObject::playbackRate() const noexcept
{
    auto value = mpv::qt::get_property(mpv, "speed");
    if (value.userType() == QMetaType::Double) return value.toFloat();

    return 1.0;
}

void MpvObject::setPlaybackRate(float playbackRate) noexcept
{
    setMpvProperty(mpv, "speed", (double)playbackRate);

    emit playbackRateChanged();
}

void MpvObject::play()
{
    setMpvProperty(mpv, "pause", false);
}

void MpvObject::pause()
{
    setMpvProperty(mpv, "pause", true);
}

void MpvObject::stop()
{
    setMpvProperty(mpv, "pause", true);
}

void MpvObject::seek(int position) noexcept
{
    setMpvProperty(mpv, "time-pos", position);
}

void MpvObject::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    auto playerEvent = mpv_wait_event(mpv, 0);
    if (playerEvent == nullptr) return;

    if(playerEvent->event_id == MPV_EVENT_FILE_LOADED) {
        qDebug() << "File Loaded!!!!";
    }
    if(playerEvent->event_id == MPV_EVENT_END_FILE) {
        qDebug() << "End file!!!!";
    }
    if(playerEvent->event_id == MPV_EVENT_COMMAND_REPLY) {
        qDebug() << "Command reply!!!!";
    }
    if(playerEvent->event_id == MPV_EVENT_PROPERTY_CHANGE) {
        auto eventData = static_cast<mpv_event_property*>(playerEvent->data);
        if (eventData == nullptr) return;
        if (eventData->format == MPV_FORMAT_NONE) return;

        auto propertyName = QString(eventData->name);

        if (propertyName == "duration") {
            auto duration = getDoubleFromEventData(eventData->data);
            m_duration = duration * 1000;
        }
        if (propertyName == "time-pos") {
            auto position = getDoubleFromEventData(eventData->data);
            m_position = position * 1000;
            qDebug() << "Position: " + QString::number(position) + " <> " + QString::number(m_position);
        }

        //eof-reached
        //seeking

        if (propertyName == "pause") m_paused = getBoolFromEventData(eventData->data);
    }
    //MPV_EVENT_GET_PROPERTY_REPLY, MPV_EVENT_SET_PROPERTY_REPLY

}

QString MpvObject::getStringFromEventData(void *data) const noexcept
{
    return QString::fromUtf8(static_cast<char*>(data));
}

qlonglong MpvObject::getInt64FromEventData(void *data) const noexcept
{
    auto value = static_cast<qlonglong*>(data);
    return *value;
}

double MpvObject::getDoubleFromEventData(void *data) const noexcept
{
    auto value = static_cast<double*>(data);
    return *value;
}

bool MpvObject::getBoolFromEventData(void *data) const noexcept
{
    auto value = *static_cast<int*>(data);
    return value == 1 ? true : false;
}

int MpvObject::setMpvProperty(mpv_handle *ctx, const QString &name, const QVariant &v)
{
    nodeBuilder node(v);
    return mpv_set_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());
}

QVariant MpvObject::getMpvProperty(mpv_handle *ctx, const QString &name)
{
    mpv_node node;
    int err = mpv_get_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, &node);
    if (err < 0) return QVariant::fromValue(ErrorReturn(err));
    nodeAutoFree f(&node);
    return nodeToVariant(&node);
}

QVariant MpvObject::nodeToVariant(const mpv_node *node)
{
    switch (node->format) {
        case MPV_FORMAT_STRING:
            return QVariant(QString::fromUtf8(node->u.string));
        case MPV_FORMAT_FLAG:
            return QVariant(static_cast<bool>(node->u.flag));
        case MPV_FORMAT_INT64:
            return QVariant(static_cast<qlonglong>(node->u.int64));
        case MPV_FORMAT_DOUBLE:
            return QVariant(node->u.double_);
        case MPV_FORMAT_NODE_ARRAY: {
            mpv_node_list *list = node->u.list;
            QVariantList qlist;
            for (int n = 0; n < list->num; n++) {
                qlist.append(nodeToVariant(&list->values[n]));
            }
            return QVariant(qlist);
        }
        case MPV_FORMAT_NODE_MAP: {
            mpv_node_list *list = node->u.list;
            QVariantMap qmap;
            for (int n = 0; n < list->num; n++) {
                qmap.insert(QString::fromUtf8(list->keys[n]), nodeToVariant(&list->values[n]));
            }
            return QVariant(qmap);
        }
        default: return QVariant();
    }
}
