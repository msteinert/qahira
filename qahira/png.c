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
#include "qahira/format/png.h"
#include "qahira/format/private.h"
#include "qahira/utility.h"

G_DEFINE_TYPE(QahiraFormatPng, qahira_format_png, QAHIRA_TYPE_FORMAT)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_FORMAT_PNG, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraFormatPng *)instance)->priv)

struct Private {
	GInputStream *input;
	GOutputStream *output;
	GCancellable *cancel;
	gboolean interlace;
	gint compression;
	GError **error;
};

static void
qahira_format_png_init(QahiraFormatPng *self)
{
	self->priv = ASSIGN_PRIVATE(self);
	struct Private *priv = GET_PRIVATE(self);
	priv->interlace = FALSE;
	priv->compression = 6;
}

static void
error_fn(png_structp png, png_const_charp message)
{
	struct Private *priv = png_get_error_ptr(png);
	if (priv->error) {
		if (*priv->error) {
			g_prefix_error(priv->error, "png: ");
		} else {
			g_set_error(priv->error, QAHIRA_ERROR,
					QAHIRA_ERROR_CORRUPT_IMAGE,
					"png: %s", message);
		}
	}
#ifdef PNG_SETJMP_SUPPORTED
	longjmp(png_jmpbuf(png), 1);
#endif // PNG_USER_MEM_SUPPORTED
}

static void
warn_fn(png_structp png, png_const_charp message)
{
	// do nothing
}

#ifdef PNG_USER_MEM_SUPPORTED
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
#endif // PNG_USER_MEM_SUPPORTED

static void
read_data_fn(png_structp png, png_bytep buffer, png_size_t size)
{
	struct Private *priv = png_get_io_ptr(png);
	while (size) {
		gssize bytes = g_input_stream_read(priv->input, buffer, size,
				priv->cancel, priv->error);
		if (-1 == bytes) {
			png_error(png, NULL);
		}
		if (size && !bytes) {
			png_error(png, "truncated file");
		}
		size -= bytes;
		buffer += bytes;
	}
}

static void
load_transform_fn(png_structp png, png_row_infop row, png_bytep data)
{
	for (gint i = 0; i < row->width; ++i) {
		gint red, green, blue;
		gint alpha = data[3];
		if (alpha == 0xff) {
			red = data[0];
			green = data[1];
			blue = data[2];
		} else {
			red = qahira_premultiply(alpha, data[0]);
			green = qahira_premultiply(alpha, data[1]);
			blue = qahira_premultiply(alpha, data[2]);
		}
		data[QAHIRA_A] = alpha;
		data[QAHIRA_R] = red;
		data[QAHIRA_G] = green;
		data[QAHIRA_B] = blue;
		data += 4;
	}
}

static cairo_surface_t *
load(QahiraFormat *self, GInputStream *stream, GCancellable *cancel,
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
#else // PNG_USER_MEM_SUPPORTED
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			priv, error_fn, warn_fn);
#endif // PNG_USER_MEM_SUPPORTED
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
#endif // PNG_SETJMP_SUPPORTED
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
#else // PNG_LIBPNG_VER
		png_set_gray_1_2_4_to_8(png);
#endif // PNG_LIBPNG_VER
		png_set_gray_to_rgb(png);
	} else if (PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png);
	}
	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png);
	}
	if (16 == depth) {
		png_set_strip_16(png);
	} else if (8 > depth) {
		png_set_packing(png);
	}
	if (PNG_INTERLACE_NONE != interlace) {
		png_set_interlace_handling(png);
	}
	png_set_filler(png, 0xff, PNG_FILLER_AFTER);
	png_read_update_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color, &interlace,
			NULL, NULL);
	if (G_UNLIKELY(8 != depth)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("png: unsupported bit depth"));
		goto error;
	}
	cairo_format_t format;
	switch (color) {
	case PNG_COLOR_TYPE_RGB:
		format = CAIRO_FORMAT_RGB24;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		format = CAIRO_FORMAT_ARGB32;
		break;
	default:
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("png: unsupported color format"));
		goto error;
	}
	png_set_read_user_transform_fn(png, load_transform_fn);
	surface = qahira_format_surface_create(self, format, width, height);
	if (G_UNLIKELY(!surface)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
	}
	cairo_status_t status = cairo_surface_status(surface);
	if (G_UNLIKELY(CAIRO_STATUS_SUCCESS != status)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_CAIRO,
				"png: %s", cairo_status_to_string(status));
		goto error;
	}
	guchar *data = qahira_format_surface_get_data(self, surface);
	if (G_UNLIKELY(!data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: surface data is NULL"));
		goto error;
	}
	gint stride = qahira_format_surface_get_stride(self, surface);
	if (G_UNLIKELY(0 > stride)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: invalid stride"));
	}
	rows = g_try_new(guchar *, height);
	if (G_UNLIKELY(!rows)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
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
write_data_fn(png_structp png, png_bytep buffer, png_size_t size)
{
	struct Private *priv = png_get_io_ptr(png);
	while (size) {
		gssize bytes = g_output_stream_write(priv->output, buffer,
				size, priv->cancel, priv->error);
		if (-1 == bytes) {
			png_error(png, NULL);
		}
		size -= bytes;
		buffer += bytes;
	}
}

static void
output_flush_fn(png_structp png)
{
	struct Private *priv = png_get_io_ptr(png);
	gboolean status = g_output_stream_flush(priv->output, priv->cancel,
			priv->error);
	if (!status) {
		png_error(png, NULL);
	}
}

static void
save_transform_fn(png_structp png, png_row_infop row, png_bytep data)
{
	for (gint i = 0; i < row->width; ++i) {
		gint red, green, blue;
		gint alpha = data[QAHIRA_A];
		if (alpha == 0xff) {
			red = data[QAHIRA_R];
			green = data[QAHIRA_G];
			blue = data[QAHIRA_B];
		} else {
			red = qahira_unpremultiply(alpha, data[QAHIRA_R]);
			green = qahira_unpremultiply(alpha, data[QAHIRA_G]);
			blue = qahira_unpremultiply(alpha, data[QAHIRA_B]);
		}
		data[0] = red;
		data[1] = green;
		data[2] = blue;
		data[3] = alpha;
		data += 4;
	}
}

static gboolean
save(QahiraFormat *self, cairo_surface_t *surface, GOutputStream *stream,
		GCancellable *cancel, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	priv->input = g_object_ref(stream);
	gboolean status = TRUE;
	png_structp png = NULL;
	png_infop info = NULL;
	png_byte **rows = NULL;
	gint width, height;
	qahira_surface_size(surface, &width, &height);
	if (!width || !height) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: invalid dimensions [%d x %d]"),
				width, height);
		goto error;
	}
#ifdef PNG_USER_MEM_SUPPORTED
	png = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, priv,
			error_fn, warn_fn, NULL, malloc_fn, free_fn);
#else // PNG_USER_MEM_SUPPORTED
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, priv,
			error_fn, warn_fn);
#endif // PNG_USER_MEM_SUPPORTED
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
	png_set_write_fn(png, priv, write_data_fn, output_flush_fn);
	priv->output = g_object_ref(stream);
	if (cancel) {
		priv->cancel = g_object_ref(cancel);
	}
	priv->error = error;
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png))) {
		goto error;
	}
#endif // PNG_SETJMP_SUPPORTED
	guchar *data = qahira_format_surface_get_data(self, surface);
	if (G_UNLIKELY(!data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: surface data is NULL"));
		goto error;
	}
	gint stride = qahira_format_surface_get_stride(self, surface);
	if (G_UNLIKELY(0 > stride)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("png: invalid stride"));
	}
	rows = g_try_new(guchar *, height);
	if (G_UNLIKELY(!rows)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("png: out of memory"));
		goto error;
	}
	for (gint i = 0; i < height; ++i) {
		rows[i] = data + i * stride;
	}
	gint depth, color;
	cairo_content_t content = cairo_surface_get_content(surface);
	switch (content) {
	case CAIRO_CONTENT_COLOR:
		depth = 8;
		color = PNG_COLOR_TYPE_RGB;
		break;
	case CAIRO_CONTENT_COLOR_ALPHA:
		depth = 8;
		color = PNG_COLOR_TYPE_RGB_ALPHA;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
		png_set_packswap(png);
#endif // G_BYTE_ORDER
		break;
	case CAIRO_CONTENT_ALPHA:
		depth = 1;
		color = PNG_COLOR_TYPE_GRAY;
		break;
	default:
		g_assert_not_reached();
	}
	png_set_compression_level(png, priv->compression);
	png_set_IHDR(png, info, width, height, depth, color,
			priv->interlace
				? PNG_INTERLACE_ADAM7
				: PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	if (PNG_COLOR_TYPE_RGB == color) {
		png_set_filler(png, 0, PNG_FILLER_AFTER);
	}
	png_set_write_user_transform_fn(png, save_transform_fn);
	png_write_image(png, rows);
	png_write_end(png, info);
exit:
	g_free(rows);
	if (png) {
		png_destroy_write_struct(&png, &info);
	}
	if (priv->output) {
		g_object_unref(priv->output);
		priv->output = NULL;
	}
	if (priv->cancel) {
		g_object_unref(priv->cancel);
		priv->cancel = NULL;
	}
	return status;
error:
	status = FALSE;
	goto exit;
}

static void
qahira_format_png_class_init(QahiraFormatPngClass *klass)
{
	QahiraFormatClass *format_class = QAHIRA_FORMAT_CLASS(klass);
	format_class->load = load;
	format_class->save = save;
	g_type_class_add_private(klass, sizeof(struct Private));
}

QahiraFormat *
qahira_format_png_new(void)
{
	return g_object_new(QAHIRA_TYPE_FORMAT_PNG,
			"mime-type", "image/png",
			NULL);
}

void
qahira_format_png_set_compression(QahiraFormat *self, gint compression)
{
	g_return_if_fail(QAHIRA_IS_FORMAT_PNG(self));
	GET_PRIVATE(self)->compression = CLAMP(compression, 0, 9);
}

gint
qahira_format_png_get_compression(QahiraFormat *self)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT_PNG(self), 0);
	return GET_PRIVATE(self)->compression;
}

void
qahira_format_set_interlace(QahiraFormat *self, gboolean interlace)
{
	g_return_if_fail(QAHIRA_IS_FORMAT_PNG(self));
	GET_PRIVATE(self)->interlace = interlace;
}

gboolean
qahira_format_get_interlace(QahiraFormat *self)
{
	g_return_val_if_fail(QAHIRA_IS_FORMAT_PNG(self), FALSE);
	return GET_PRIVATE(self)->interlace;
}
