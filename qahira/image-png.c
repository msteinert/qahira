/* Copyright 2003 University of Southern California
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
#include <png.h>
#include "qahira/error.h"
#include "qahira/image/png.h"
#include "qahira/image/private.h"

G_DEFINE_TYPE(QahiraImagePng, qahira_image_png, QAHIRA_TYPE_IMAGE)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_IMAGE_PNG, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraImagePng *)instance)->priv)

struct Private {
	GInputStream *input;
	GCancellable *cancel;
	GError **error;
};

static void
qahira_image_png_init(QahiraImagePng *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
dispose(GObject *base)
{
	//struct Private *priv = GET_PRIVATE(base);
	G_OBJECT_CLASS(qahira_image_png_parent_class)->dispose(base);
}

static void
finalize(GObject *base)
{
	//struct Private *priv = GET_PRIVATE(base);
	G_OBJECT_CLASS(qahira_image_png_parent_class)->finalize(base);
}

static void
error_fn(png_structp png, png_const_charp message)
{
	struct Private *priv = png_get_error_ptr(png);
	g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_CORRUPT_IMAGE,
			Q_("png: %s"), message);
#ifdef PNG_SETJMP_SUPPORTED
	longjmp(png_jmpbuf(png), 1);
#endif
}

static void
warn_fn(png_structp png, png_const_charp message)
{
	// do nothing
}

static png_voidp
malloc_fn(png_structp png, png_size_t size)
{
	return g_try_malloc(size);
}

static void
free_fn(png_structp png, png_voidp ptr)
{
	g_free(ptr);
}

static void
read_data_fn(png_structp png, png_bytep buffer, png_size_t size)
{
	struct Private *priv = png_get_io_ptr(png);
	while (size) {
		gssize bytes = g_input_stream_read(priv->input, buffer, size,
				priv->cancel, priv->error);
		if (-1 == bytes) {
			priv->error = NULL;
			png_error(png, NULL);
		}
		if (size && !bytes) {
			png_error(png, "truncated file");
		}
		size -= bytes;
		buffer += bytes;
	}
}

static inline gint
multiply_alpha(gint alpha, gint color)
{
	gint tmp = alpha * color + 0x80;
	return ((tmp + (tmp >> 8)) >> 8);
}

static void
premultiply_transform_fn(png_structp png, png_row_infop row, png_bytep data)
{
	for (gint i = 0; i < row->rowbytes; i += 4) {
		guchar *base = &data[i];
		guchar alpha = base[3];
		guint pixel;
		if (!alpha) {
			pixel = 0;
		} else {
			guchar red = base[0];
			guchar green = base[1];
			guchar blue = base[2];
			if (alpha != 0xff) {
				red = multiply_alpha(alpha, red);
				green = multiply_alpha(alpha, green);
				blue = multiply_alpha(alpha, blue);
			}
			pixel = (alpha << 24) | (red << 16) | (green << 8)
				| (blue << 0);
		}
		memcpy(base, &pixel, sizeof(guint));
	}
}

static void
convert_transform_fn(png_structp png, png_row_infop row, png_bytep data)
{
	for (gint i = 0; i < row->rowbytes; i += 4) {
		guchar *base = &data[i];
		guchar red = base[0];
		guchar green = base[1];
		guchar blue = base[2];
		guint pixel = (0xff << 24)  | (red << 16) | (green < 8)
			| (blue << 0);
		memcpy(base, &pixel, sizeof(guint));
	}
}

static cairo_surface_t *
load(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	cairo_surface_t *surface = NULL;
	png_byte **rows = NULL;
	png_structp png = NULL;
	png_infop info = NULL;
#ifdef PNG_USER_MEM_SUPPORTED
	png = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
			priv, error_fn, warn_fn, NULL, malloc_fn, free_fn);
#else
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			priv, error_fn, warn_fn);
#endif
	if (G_UNLIKELY(!png)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
	}
	info = png_create_info_struct(png);
	if (G_UNLIKELY(!info)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
	}
	png_set_read_fn(png, priv, read_data_fn);
	priv->input = g_object_ref(stream);
	if (cancel) {
		priv->cancel = g_object_ref(cancel);
	}
	priv->error = error;
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png))) {
		goto error;
	}
#endif
	png_uint_32 width, height;
	int depth, color, interlace;
	png_read_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color,
			&interlace, NULL, NULL);
	if (PNG_COLOR_TYPE_PALETTE == color) {
		png_set_palette_to_rgb(png);
	} else if (PNG_COLOR_TYPE_GRAY == color) {
#if PNG_LIBPNG_VER >= 10209
		png_set_expand_gray_1_2_4_to_8(png);
#else
		png_set_gray_1_2_4_to_8(png);
#endif
		png_set_gray_to_rgb(png);
	} else if (PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png);
	}
	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png);
	}
	if (16 == depth) {
		png_set_strip_16(png);
	}
	if (8 > depth) {
		png_set_packing(png);
	}
	if (PNG_INTERLACE_NONE != interlace) {
		png_set_interlace_handling(png);
	}
	png_set_filler(png, 0xff, PNG_FILLER_AFTER);
	png_read_update_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color,
			&interlace, NULL, NULL);
	if (G_UNLIKELY(8 != depth)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_CORRUPT_IMAGE,
				Q_("png: unsupported bit depth"));
		goto error;
	}
	cairo_format_t format;
	switch (color) {
	case PNG_COLOR_TYPE_RGB:
		format = CAIRO_FORMAT_ARGB32;
		png_set_read_user_transform_fn(png,
				premultiply_transform_fn);
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		format = CAIRO_FORMAT_RGB24;
		png_set_read_user_transform_fn(png,
				convert_transform_fn);
		break;
	default:
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_CORRUPT_IMAGE,
				Q_("png: unsupported color format"));
		goto error;
	}
	surface = qahira_image_surface_create(self, format, width, height);
	if (G_UNLIKELY(!surface)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
	}
	cairo_status_t status = cairo_surface_status(surface);
	if (G_UNLIKELY(CAIRO_STATUS_SUCCESS != status)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_CAIRO,
				cairo_status_to_string(status));
		goto error;
	}
	rows = g_try_new(guchar *, height);
	if (G_UNLIKELY(!rows)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
	}
	guchar *data = qahira_image_surface_get_data(self, surface);
	if (G_UNLIKELY(!data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: data is NULL"));
		goto error;
	}
	gint stride = qahira_image_surface_get_stride(self, surface);
	if (G_UNLIKELY(0 > stride)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: invalid stride"));
	}
	for (gint i = 0; i < height; ++i) {
		rows[i] = &data[i * stride];
	}
	cairo_surface_flush(surface);
	png_read_image(png, rows);
	png_read_end(png, info);
	cairo_surface_mark_dirty(surface);
exit:
	g_free(rows);
	if (png) {
		png_destroy_read_struct(&png, &info, NULL);
		png = NULL;
	}
	if (priv->input) {
		g_object_unref(priv->input);
		priv->input = NULL;
	}
	if (priv->cancel) {
		g_object_unref(priv->cancel);
		priv->cancel = NULL;
	}
	return surface;
error:
	if (surface) {
		cairo_surface_destroy(surface);
		surface = NULL;
	}
	goto exit;
}

static void
qahira_image_png_class_init(QahiraImagePngClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	QahiraImageClass *image_class = QAHIRA_IMAGE_CLASS(klass);
	image_class->load = load;
	g_type_class_add_private(klass, sizeof(struct Private));
}

QahiraImage *
qahira_image_png_new(void)
{
	return g_object_new(QAHIRA_TYPE_IMAGE_PNG,
			"mime-type", "image/png",
			NULL);
}