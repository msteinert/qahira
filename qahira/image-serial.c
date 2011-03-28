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
#include "qahira/error.h"
#include "qahira/image/serial.h"
#include "qahira/image/private.h"

G_DEFINE_TYPE(QahiraImageSerial, qahira_image_serial, QAHIRA_TYPE_IMAGE)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_IMAGE_SERIAL, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraImageSerial *)instance)->priv)

typedef struct SerialHeader_ {
	cairo_content_t content;
	gint width;
	gint height;
	gint stride;
} SerialHeader;

struct Private {
	SerialHeader header;
};

static void
qahira_image_serial_init(QahiraImageSerial *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
finalize(GObject *base)
{
	//struct Private *priv = GET_PRIVATE(base);
	G_OBJECT_CLASS(qahira_image_serial_parent_class)->finalize(base);
}

static gboolean
serial_read(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		guchar *buffer, gsize size, GError **error)
{
	while (size) {
		gssize bytes = g_input_stream_read(stream, buffer, size,
				cancel, error);
		if (G_UNLIKELY(-1 == bytes)) {
			return FALSE;
		}
		if (G_UNLIKELY(size && !bytes)) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_CORRUPT_IMAGE,
					Q_("serial: truncated image"));
			return FALSE;
		}
		size -= bytes;
		buffer += bytes;
	}
	return TRUE;
}

static cairo_surface_t *
load(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	cairo_surface_t *surface = NULL;
	gboolean status = serial_read(self, stream, cancel,
			(gpointer)&priv->header, sizeof(priv->header), error);
	if (!status) {
		goto error;
	}
	cairo_format_t format;
	switch (priv->header.content) {
	case CAIRO_CONTENT_COLOR:
		format = CAIRO_FORMAT_RGB24;
		break;
	case CAIRO_CONTENT_COLOR_ALPHA:
		format = CAIRO_FORMAT_ARGB32;
		break;
	case CAIRO_CONTENT_ALPHA:
		format = CAIRO_FORMAT_A8;
		break;
	default:
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("serial: unsupported content type"));
		goto error;
	}
	surface = qahira_image_surface_create(self, format,
			priv->header.width, priv->header.height);
	if (!surface) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("serial: out of memory"));
		goto error;
	}
	guchar *data = qahira_image_surface_get_data(self, surface);
	if (!data) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("serial: surface data is NULL"));
		goto error;
	}
	gint stride = qahira_image_surface_get_stride(self, surface);
	if (stride != priv->header.stride) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("serial: invalid stride"));
		goto error;
	}
	cairo_surface_flush(surface);
	status = serial_read(self, stream, cancel, data,
			priv->header.height * stride, error);
	if (!status) {
		goto error;
	}
	cairo_surface_mark_dirty(surface);
exit:
	return surface;
error:
	if (surface) {
		cairo_surface_destroy(surface);
		surface = NULL;
	}
	goto exit;
}

static gboolean
serial_write(QahiraImage *self, GOutputStream *stream, GCancellable *cancel,
		guchar *buffer, gsize size, GError **error)
{
	while (size) {
		gssize bytes = g_output_stream_write(stream, buffer, size,
				cancel, error);
		if (G_UNLIKELY(-1 == bytes)) {
			return FALSE;
		}
		size -= bytes;
		buffer += bytes;
	}
	return TRUE;

}

static gboolean
save(QahiraImage *self, cairo_surface_t *surface, GOutputStream *stream,
		GCancellable *cancel, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	gboolean status = TRUE;
	guchar *data = qahira_image_surface_get_data(self, surface);
	if (!data) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("serial: surface data is NULL"));
		goto error;
	}
	priv->header.stride = qahira_image_surface_get_stride(self, surface);
	if (0 > priv->header.stride) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("serial: invalid stride"));
		goto error;
	}
	qahira_image_surface_size(surface, &priv->header.width,
			&priv->header.height);
	priv->header.content = cairo_surface_get_content(surface);
	status = serial_write(self, stream, cancel, (gpointer)&priv->header,
			sizeof(priv->header), error);
	if (!status) {
		goto error;
	}
	status = serial_write(self, stream, cancel, data,
			priv->header.stride * priv->header.height, error);
exit:
	return status;
error:
	status = FALSE;
	goto exit;
}

static void
qahira_image_serial_class_init(QahiraImageSerialClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = finalize;
	QahiraImageClass *image_class = QAHIRA_IMAGE_CLASS(klass);
	image_class->load = load;
	image_class->save = save;
	g_type_class_add_private(klass, sizeof(struct Private));
}

QahiraImage *
qahira_image_serial_new(void)
{
	return g_object_new(QAHIRA_TYPE_IMAGE_SERIAL,
			"mime-type", "application/octet-stream",
			NULL);
}

gsize
qahira_image_serial_get_size(QahiraImage *self, cairo_surface_t *surface)
{
	gint stride = qahira_image_surface_get_stride(self, surface);
	if (G_UNLIKELY(0 > stride)) {
		return 0;
	}
	gint height;
	qahira_image_surface_size(surface, NULL, &height);
	return sizeof(GET_PRIVATE(self)->header) + stride * height;
}
