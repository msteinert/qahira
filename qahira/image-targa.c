/* Copyright 2001, 2002  Matthias Brueckner
 * Copyright 2011 Michael Steinert
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
#include "qahira/image/targa.h"
#include "qahira/image/private.h"

G_DEFINE_TYPE(QahiraImageTarga, qahira_image_targa, QAHIRA_TYPE_IMAGE)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_IMAGE_TARGA, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraImageTarga *)instance)->priv)

#define TGA_HEADER_SIZE (18)

typedef struct TargaHeader_ {
	guchar id_len; // image id length
	guchar map_t; // color map type
	guchar img_t; // image type
	gshort map_first; // index of first map entry
	gshort map_len; // number of entries in color map
	guchar map_entry; // bit-depth of a cmap entry
	gshort x; // x-coordinate
	gshort y; // y-coordinate
	gshort width; // width of image
	gshort height; // height of image
	guchar depth; // pixel-depth of image
	guchar alpha; // alpha bits
	guchar horz;// horizontal orientation
	guchar vert; // vertical orientation
} TargaHeader;

struct Private {
	TargaHeader header;
	guchar *colormap;
	GError **error;
	guchar *buffer;
	gsize position;
	gsize size;
	guchar *id;
};

static void
qahira_image_targa_init(QahiraImageTarga *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
dispose(GObject *base)
{
	//struct Private *priv = GET_PRIVATE(base);
	G_OBJECT_CLASS(qahira_image_targa_parent_class)->dispose(base);
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	g_free(priv->colormap);
	g_free(priv->buffer);
	g_free(priv->id);
	G_OBJECT_CLASS(qahira_image_targa_parent_class)->finalize(base);
}

static gboolean
tga_read(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		guchar *buffer, gsize size, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	while (size) {
		gssize bytes = g_input_stream_read(stream, buffer, size,
				cancel, error);
		if (-1 == bytes) {
			return FALSE;
		}
		if (size && !bytes) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_CORRUPT_IMAGE,
					Q_("truncated image"));
			return FALSE;
		}
		priv->position += bytes;
		size -= bytes;
		buffer += bytes;
	}
	return TRUE;
}

static gboolean
tga_read_rle(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		guchar *buffer, gsize size, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	gint bpp;
	switch (priv->header.depth) {
	case 15:
 	case 16:
		bpp = 2;
		break;
	case 24:
		bpp = 3;
		break;
	case 32:
		bpp = 4;
		break;
	}
	guchar pixel[4];
	gboolean status;
	gint repeat = 0, direct = 0;
	for (gint i = 0; i < priv->header.width; ++i) {
		if (!repeat && !direct) {
			status = tga_read(self, stream, cancel, priv->buffer,
					1, error);
			if (!status) {
				return FALSE;
			}
			if (128 <= priv->buffer[0]) {
				repeat = priv->buffer[0] - 127;
				status = tga_read(self, stream, cancel, pixel,
						bpp, error);
				if (!status) {
					return FALSE;
				}
			} else {
				direct = priv->buffer[0];
			}
		}
		if (0 < repeat) {
			for (gint j = 0; j < bpp; ++j) {
				priv->buffer[j] = pixel[j];
				--repeat;
			}
		} else {
			status = tga_read(self, stream, cancel, priv->buffer,
					bpp, error);
			if (!status) {
				return FALSE;
			}
			--direct;
		}
		buffer += bpp;
	}
	return TRUE;
}

static gboolean
tga_skip(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		gsize size, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	if (priv->position > size) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("seek error"));
		return FALSE;
	}
	size = size - priv->position;
	while (size) {
		gssize bytes = g_input_stream_skip(stream, size, cancel,
				error);
		if (-1 == bytes) {
			return FALSE;
		}
		if (size && !bytes) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_CORRUPT_IMAGE,
					Q_("truncated image"));
			return FALSE;
		}
		priv->position += bytes;
		size -= bytes;
	}
	return TRUE;
}

static inline gboolean
ensure_buffer(QahiraImage *self, gsize size, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	if (priv->size < size) {
		g_free(priv->buffer);
		priv->buffer = g_try_malloc(size);
		if (G_UNLIKELY(!priv->buffer)) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_NO_MEMORY,
					Q_("targa: out of memory"));
			return FALSE;
		}
		priv->size = size;
	}
	return TRUE;
}

static gboolean
read_header(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	if (!ensure_buffer(self, TGA_HEADER_SIZE, error)) {
		return FALSE;
	}
	gboolean status = tga_read(self, stream, cancel, priv->buffer,
			TGA_HEADER_SIZE, error);
	if (!status) {
		return FALSE;
	}
	priv->header.id_len = priv->buffer[0];
	priv->header.map_t = priv->buffer[1];
	priv->header.img_t = priv->buffer[2];
	priv->header.map_first = priv->buffer[3] + priv->buffer[4] * 256;
	priv->header.map_len = priv->buffer[5] + priv->buffer[6] * 256;
	priv->header.map_entry = priv->buffer[7];
	priv->header.x = priv->buffer[8] + priv->buffer[9] * 256;
	priv->header.y = priv->buffer[10] + priv->buffer[11] * 256;
	priv->header.width = priv->buffer[12] + priv->buffer[13] * 256;
	priv->header.height = priv->buffer[14] + priv->buffer[15] * 256;
	priv->header.depth = priv->buffer[16];
	priv->header.alpha = priv->buffer[17] & 0x0f;
	priv->header.horz = (priv->buffer[17] & 0x10)
		? QAHIRA_ORIENTATION_TOP
		: QAHIRA_ORIENTATION_BOTTOM;
	priv->header.vert = (priv->buffer[17] & 0x20)
		? QAHIRA_ORIENTATION_RIGHT
		: QAHIRA_ORIENTATION_LEFT;
	if (priv->header.map_t && priv->header.depth != 8) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("targa: wrong bit depth for colormap: %d"),
				priv->header.depth);
		return FALSE;
	}
	switch (priv->header.depth) {
	case 8:
	case 15:
	case 16:
	case 24:
	case 32:
		break;
	default:
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("targa: unsupported bit depth: %d"),
				priv->header.depth);
		return FALSE;
	}
	return TRUE;
}

static gboolean
read_image_id(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);

	gboolean status = tga_skip(self, stream, cancel, TGA_HEADER_SIZE,
			error);
	if (!status) {
		return FALSE;
	}
	priv->id = g_try_malloc(priv->header.id_len);
	if (G_UNLIKELY(!priv->buffer)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("targa: out of memory"));
		return FALSE;
	}
	return tga_read(self, stream, cancel, priv->id, priv->header.id_len,
			error);
}

static gboolean
read_colormap(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	gsize size = priv->header.map_len * priv->header.map_entry / 8;
	if (!size) {
		return TRUE;
	}
	gboolean status = tga_skip(self, stream, cancel, TGA_HEADER_SIZE,
			error);
	if (!status) {
		return FALSE;
	}
	priv->colormap = g_try_malloc(size);
	if (G_UNLIKELY(!priv->colormap)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("targa: out of memory"));
		return FALSE;
	}
	return tga_read(self, stream, cancel, priv->colormap, size, error);
}

static inline void
convert_rgb(QahiraImage *self, const guchar *in, guchar *out)
{
	struct Private *priv = GET_PRIVATE(self);
	for (gint i = 0; i < priv->header.width; ++i) {
		switch (priv->header.depth) {
		case 15:
			{
				gushort pixel = (in[0] << 8) | in[1];
				out[QAHIRA_R] = (pixel >> 10) & 0x1f;
				out[QAHIRA_G] = (pixel >> 5) & 0x1f;
				out[QAHIRA_B] = pixel & 0x1f;
				if (priv->header.alpha) {
					out[QAHIRA_A] =
						(pixel >> 15) & 0x1 ? 1 : 0;
				}
			}
			in += 2;
			break;
		case 16:
			{
				gushort pixel = (in[0] << 8) | in[1];
				out[QAHIRA_R] = (pixel >> 11) & 0x1f;
				out[QAHIRA_G] = (pixel >> 5) & 0x3f;
				out[QAHIRA_B] = pixel & 0x1f;
			}
			in += 2;
			break;
		case 24:
			out[QAHIRA_R] = in[0];
			out[QAHIRA_G] = in[1];
			out[QAHIRA_B] = in[2];
			in += 3;
			break;
		case 32:
			out[QAHIRA_R] = in[0];
			out[QAHIRA_G] = in[1];
			out[QAHIRA_B] = in[2];
			out[QAHIRA_A] = in[3];
			in += 4;
			break;
		default:
			g_assert_not_reached();
		}
		out += 4;
	}
}

static cairo_surface_t *
read_scanlines(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	cairo_surface_t *surface = NULL;
	gsize offset = TGA_HEADER_SIZE + priv->header.id_len
		+ (priv->header.map_len * priv->header.map_entry / 8);
	gboolean status = tga_skip(self, stream, cancel, offset, error);
	if (!status) {
		goto error;
	}
	cairo_format_t format;
	switch (priv->header.depth) {
	case 16:
	case 24:
		format = CAIRO_FORMAT_RGB24;
		break;
	case 15:
		format = priv->header.alpha ? CAIRO_FORMAT_ARGB32
			: CAIRO_FORMAT_RGB24;
		break;
	case 32:
		format = CAIRO_FORMAT_ARGB32;
		break;
	default:
		g_assert_not_reached();
	}
	surface = qahira_image_surface_create(self, format,
			priv->header.width, priv->header.height);
	if (G_UNLIKELY(!surface)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("targa: out of memory"));
		goto error;
	}
	guchar *data = qahira_image_surface_get_data(self, surface);
	if (G_UNLIKELY(!data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("targa: data is NULL"));
		goto error;
	}
	cairo_surface_flush(surface);
	gint stride = priv->header.width * priv->header.depth / 8;
	if (!ensure_buffer(self, stride, error)) {
		goto error;
	}
	gint cairo_stride = qahira_image_surface_get_stride(self, surface);
	if (G_UNLIKELY(0 > cairo_stride)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("targa: invalid stride"));
		goto error;
	}
	if (priv->header.img_t > 8 && priv->header.img_t < 12) {
		for (gint i = 0; i < priv->header.height; ++i) {
			status = tga_read_rle(self, stream, cancel, data,
					stride, error);
			if (!status) {
				goto error;
			}
			convert_rgb(self, priv->buffer, data);
			data += cairo_stride;
		}
	} else {
		for (gint i = 0; i < priv->header.height; ++i) {
			status = tga_read(self, stream, cancel, priv->buffer,
					stride, error);
			if (!status) {
				goto error;
			}
			convert_rgb(self, priv->buffer, data);
			data += cairo_stride;
		}
	}
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

static cairo_surface_t *
load(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	cairo_surface_t *surface = NULL;
	if (!read_header(self, stream, cancel, error)) {
		goto exit;
	}
	if (priv->header.id_len) {
		if (!read_image_id(self, stream, cancel, error)) {
			goto exit;
		}
	} else {
		g_free(priv->id);
		priv->id = NULL;
	}
	if (priv->header.map_t) {
		if (!read_colormap(self, stream, cancel, error)) {
			goto exit;
		}
	} else {
		g_free(priv->colormap);
		priv->colormap = NULL;
	}
	surface = read_scanlines(self, stream, cancel, error);
exit:
	return surface;
}

static void
qahira_image_targa_class_init(QahiraImageTargaClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	QahiraImageClass *image_class = QAHIRA_IMAGE_CLASS(klass);
	image_class->load = load;
	g_type_class_add_private(klass, sizeof(struct Private));
}

QahiraImage *
qahira_image_targa_new(void)
{
	return g_object_new(QAHIRA_TYPE_IMAGE_TARGA,
			"mime-type", "image/x-targa",
			"mime-type", "image/x-tga",
			NULL);
}
