/*
 *  gstvaapiobject.c - Base VA object
 *
 *  gstreamer-vaapi (C) 2010 Splitted-Desktop Systems
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "config.h"
#include "gstvaapiobject.h"
#include "gstvaapimarshal.h"

#define DEBUG 1
#include "gstvaapidebug.h"

G_DEFINE_TYPE(GstVaapiObject, gst_vaapi_object, G_TYPE_OBJECT);

#define GST_VAAPI_OBJECT_GET_PRIVATE(obj)                       \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj),                         \
                                 GST_VAAPI_TYPE_OBJECT,         \
                                 GstVaapiObjectPrivate))

struct _GstVaapiObjectPrivate {
    GstVaapiDisplay    *display;
    guint               is_destroying   : 1;
};

enum {
    PROP_0,

    PROP_DISPLAY,
};

enum {
    DESTROY,

    LAST_SIGNAL
};

static guint object_signals[LAST_SIGNAL] = { 0, };

static void
gst_vaapi_object_dispose(GObject *object)
{
    GstVaapiObjectPrivate * const priv = GST_VAAPI_OBJECT(object)->priv;

    if (!priv->is_destroying) {
        priv->is_destroying = TRUE;
        g_signal_emit(object, object_signals[DESTROY], 0);
        priv->is_destroying = FALSE;
    }

    G_OBJECT_CLASS(gst_vaapi_object_parent_class)->dispose(object);
}

static void
gst_vaapi_object_finalize(GObject *object)
{
    GstVaapiObjectPrivate * const priv = GST_VAAPI_OBJECT(object)->priv;

    if (priv->display) {
        g_object_unref(priv->display);
        priv->display = NULL;
    }

    G_OBJECT_CLASS(gst_vaapi_object_parent_class)->finalize(object);
}

static void
gst_vaapi_object_set_property(
    GObject      *gobject,
    guint         prop_id,
    const GValue *value,
    GParamSpec   *pspec
)
{
    GstVaapiObject * const object = GST_VAAPI_OBJECT(gobject);

    switch (prop_id) {
    case PROP_DISPLAY:
        object->priv->display = g_object_ref(g_value_get_object(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}

static void
gst_vaapi_object_get_property(
    GObject    *gobject,
    guint       prop_id,
    GValue     *value,
    GParamSpec *pspec
)
{
    GstVaapiObject * const object = GST_VAAPI_OBJECT(gobject);

    switch (prop_id) {
    case PROP_DISPLAY:
        g_value_set_object(value, gst_vaapi_object_get_display(object));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}

static void
gst_vaapi_object_class_init(GstVaapiObjectClass *klass)
{
    GObjectClass * const object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(GstVaapiObjectPrivate));

    object_class->dispose      = gst_vaapi_object_dispose;
    object_class->finalize     = gst_vaapi_object_finalize;
    object_class->set_property = gst_vaapi_object_set_property;
    object_class->get_property = gst_vaapi_object_get_property;

    /**
     * GstVaapiObject:display:
     *
     * The #GstVaapiDisplay this object is bound to.
     */
    g_object_class_install_property
        (object_class,
         PROP_DISPLAY,
         g_param_spec_object("display",
                             "Display",
                             "The GstVaapiDisplay this object is bound to",
                             GST_VAAPI_TYPE_DISPLAY,
                             G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));

    /**
     * GstVaapiObject::destroy:
     * @object: the object which received the signal
     *
     * The ::destroy signal is emitted when an object is destroyed,
     * when the user released the last reference to @object.
     */
    object_signals[DESTROY] = g_signal_new(
        "destroy",
        G_TYPE_FROM_CLASS(object_class),
        G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET(GstVaapiObjectClass, destroy),
        NULL, NULL,
        gst_vaapi_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );
}

static void
gst_vaapi_object_init(GstVaapiObject *object)
{
    GstVaapiObjectPrivate *priv = GST_VAAPI_OBJECT_GET_PRIVATE(object);

    object->priv  = priv;
    priv->display = NULL;
    priv->is_destroying = FALSE;
}

/**
 * gst_vaapi_object_get_display:
 * @object: a #GstVaapiObject
 *
 * Returns the #GstVaapiDisplay this @object is bound to.
 *
 * Return value: the parent #GstVaapiDisplay object
 */
GstVaapiDisplay *
gst_vaapi_object_get_display(GstVaapiObject *object)
{
    g_return_val_if_fail(GST_VAAPI_IS_OBJECT(object), NULL);

    return object->priv->display;
}