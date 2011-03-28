/* Copyright 2011 Michael Steinert
 * This file is part of Qahira.
 *
 * Qahira is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * Qahira is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Qahira. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "qahira/accumulator.h"
#include "qahira/format/private.h"
#include "qahira/macros.h"
#include "qahira/marshal.h"

G_DEFINE_ABSTRACT_TYPE(QahiraFormat, qahira_format, G_TYPE_OBJECT)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_FORMAT, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraFormat *)instance)->priv)

enum Signals {
	SIGNAL_SURFACE_CREATE,
	SIGNAL_SURFACE_GET_DATA,
	SIGNAL_SURFACE_GET_STRIDE,
	SIGNAL_LAST
};

static guint signals[SIGNAL_LAST] = { 0 };

struct Private {
	GSList *types;
};

static void
qahira_format_init(QahiraFormat *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->types) {
		g_slist_free(priv->types);
	}
	G_OBJECT_CLASS(qahira_format_parent_class)->finalize(base);
}

enum Properties {
	PROP_O = 0,
	PROP_MIME_TYPE,
	PROP_MAX
};

static void
set_property(GObject *base, guint id, const GValue *value, GParamSpec *pspec)
{
	QahiraFormat *self = (QahiraFormat *)base;
	switch (id) {
	case PROP_MIME_TYPE:
		qahira_format_add_type(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(base, id, pspec);
		break;
	}
}

static cairo_surface_t *
load(QahiraFormat *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
			Q_("load operation is not supported"));
	return NULL;
}

static gboolean
save(QahiraFormat *self, cairo_surface_t *surface, GOutputStream *stream,
		GCancellable *cancel, GError **error)
{
	g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
			Q_("save operation is not supported"));
	return FALSE;
}

static cairo_surface_t *
surface_create(QahiraFormat *self, cairo_format_t format,
		gint width, gint height)
{
	return cairo_image_surface_create(format, width, height);
}

static guchar *
surface_get_data(QahiraFormat *self, cairo_surface_t *surface)
{
	cairo_surface_type_t type = cairo_surface_get_type(surface);
	if (CAIRO_SURFACE_TYPE_IMAGE != type) {
		return NULL;
	}
	return cairo_image_surface_get_data(surface);
}

static gint
surface_get_stride(QahiraFormat *self, cairo_surface_t *surface)
{
	cairo_surface_type_t type = cairo_surface_get_type(surface);
	if (CAIRO_SURFACE_TYPE_IMAGE != type) {
		return -1;
	}
	return cairo_image_surface_get_stride(surface);
}

static void
qahira_format_class_init(QahiraFormatClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = finalize;
	object_class->set_property = set_property;
	klass->load = load;
	klass->save = save;
	klass->surface_create = surface_create;
	klass->surface_get_data = surface_get_data;
	klass->surface_get_stride = surface_get_stride;
	g_type_class_add_private(klass, sizeof(struct Private));
	// QahiraFormat::surface-create
	signals[SIGNAL_SURFACE_CREATE] =
		g_signal_new(g_intern_static_string("surface-create"),
			G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(QahiraFormatClass, surface_create),
			qahira_pointer_accumulator, NULL,
			qahira_marshal_POINTER__INT_INT_INT, G_TYPE_POINTER,
			3, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
	// QahiraFormat::surface-get-data
	signals[SIGNAL_SURFACE_GET_DATA] =
		g_signal_new(g_intern_static_string("surface-get-data"),
			G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(QahiraFormatClass, surface_get_data),
			qahira_pointer_accumulator, NULL,
			qahira_marshal_POINTER__POINTER, G_TYPE_POINTER,
			1, G_TYPE_POINTER);
	// QahiraFormat::surface-get-stride
	signals[SIGNAL_SURFACE_GET_STRIDE] =
		g_signal_new(g_intern_static_string("surface-get-stride"),
			G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(QahiraFormatClass, surface_get_stride),
			qahira_integer_accumulator, NULL,
			qahira_marshal_INT__POINTER, G_TYPE_INT,
			1, G_TYPE_POINTER);
	// properties
	g_object_class_install_property(object_class, PROP_MIME_TYPE,
		g_param_spec_string("mime-type", Q_("MIME type"),
			Q_("MIME type(s) of this format format"), NULL,
			G_PARAM_WRITABLE));
}

cairo_surface_t *
qahira_format_load(QahiraFormat *self, GInputStream *stream,
		GCancellable *cancel, GError **error)
{
	qahira_return_error_if_fail(QAHIRA_IS_FORMAT(self), NULL, error);
	qahira_return_error_if_fail(G_IS_INPUT_STREAM(stream), NULL, error);
	return QAHIRA_FORMAT_GET_CLASS(self)->
		load(self, stream, cancel, error);
}

gboolean
qahira_format_save(QahiraFormat *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error)
{
	qahira_return_error_if_fail(QAHIRA_IS_FORMAT(self), FALSE, error);
	qahira_return_error_if_fail(surface, FALSE, error);
	qahira_return_error_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE, error);
	return QAHIRA_FORMAT_GET_CLASS(self)->
		save(self, surface, stream, cancel, error);
}

gboolean
qahira_format_supports(QahiraFormat *self, const gchar *type)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT(self), FALSE);
	g_return_val_if_fail(type, FALSE);
	const gchar *string = g_intern_string(type);
	if (G_UNLIKELY(!string)) {
		return FALSE;
	}
	return qahira_format_supports_intern_string(self, string);
}

gboolean
qahira_format_supports_intern_string(QahiraFormat *self, const gchar *type)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT(self), FALSE);
	g_return_val_if_fail(type, FALSE);
	struct Private *priv = GET_PRIVATE(self);
	for (GSList *node = priv->types; node; node = node->next) {
		if (type == node->data) {
			return TRUE;
		}
	}
	return FALSE;
}

void
qahira_format_add_type(QahiraFormat *self, const gchar *type)
{
	g_return_if_fail(QAHIRA_IS_FORMAT(self));
	g_return_if_fail(type);
	struct Private *priv = GET_PRIVATE(self);
	const gchar *string = g_intern_string(type);
	if (G_UNLIKELY(!string)) {
		return;
	}
	priv->types = g_slist_prepend(priv->types, (gpointer)string);
}

void
qahira_format_add_static_type(QahiraFormat *self, const gchar *type)
{
	g_return_if_fail(QAHIRA_IS_FORMAT(self));
	g_return_if_fail(type);
	struct Private *priv = GET_PRIVATE(self);
	const gchar *string = g_intern_static_string(type);
	if (G_UNLIKELY(!string)) {
		return;
	}
	priv->types = g_slist_prepend(priv->types, (gpointer)string);
}

cairo_surface_t *
qahira_format_surface_create(QahiraFormat *self, cairo_format_t format,
		gint width, gint height)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT(self), NULL);
	cairo_surface_t *surface;
	g_signal_emit(self, signals[SIGNAL_SURFACE_CREATE], 0,
			format, width, height, &surface);
	return surface;
}

guchar *
qahira_format_surface_get_data(QahiraFormat *self, cairo_surface_t *surface)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT(self), NULL);
	g_return_val_if_fail(surface, NULL);
	guchar *data;
	g_signal_emit(self, signals[SIGNAL_SURFACE_GET_DATA], 0,
			surface, &data);
	return data;
}

gint
qahira_format_surface_get_stride(QahiraFormat *self, cairo_surface_t *surface)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT(self), -1);
	g_return_val_if_fail(surface, -1);
	gint stride;
	g_signal_emit(self, signals[SIGNAL_SURFACE_GET_STRIDE], 0,
			surface, &stride);
	return stride;
}

void
qahira_format_surface_size(cairo_surface_t *surface, gint *width,
		gint *height)
{
	cairo_t *cr = cairo_create(surface);
	gdouble clip_width, clip_height;
	cairo_clip_extents(cr, NULL, NULL, &clip_width, &clip_height);
	cairo_destroy(cr);
	if (width) {
		*width = (gint)clip_width;
	}
	if (height) {
		*height = (gint)clip_height;
	}
}
