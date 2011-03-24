/* Copyright 1999 Michael Zucchi
 * Copyright 1999 The Free Software Foundation
 * Copyright 1999 Red Hat, Inc.
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
#include "qahira/image/jpeg.h"
#include "qahira/image/private.h"
#include "qahira/marshal.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <jerror.h>

G_DEFINE_TYPE(QahiraImageJpeg, qahira_image_jpeg, QAHIRA_TYPE_IMAGE)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_IMAGE_JPEG, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraImageJpeg *)instance)->priv)

enum Signals {
	SIGNAL_PROGRESSIVE,
	SIGNAL_LAST
};

static guint signals[SIGNAL_LAST] = { 0 };

#define QAHIRA_JPEG_BUFFER_SIZE (1024 * 32)

struct Private {
	struct jpeg_decompress_struct decompress;
	struct jpeg_source_mgr source_mgr;
	struct jpeg_error_mgr error_mgr;
	GInputStream *input;
	GCancellable *cancel;
	JOCTET *buffer;
	gsize size;
	guchar **lines;
	gsize count;
	cairo_surface_t *surface;
	guchar *data;
	gint stride;
	GError **error;
	sigjmp_buf env;
	gchar message[JMSG_LENGTH_MAX];
};

/**
 * \brief Convert JPEG a error to a GError.
 */
G_GNUC_NORETURN
static void
error_exit(j_common_ptr cinfo)
{
	struct Private *priv = cinfo->client_data;
	cinfo->err->format_message(cinfo, priv->message);
	g_set_error(priv->error, QAHIRA_ERROR,
			cinfo->err->msg_code == JERR_OUT_OF_MEMORY
				? QAHIRA_ERROR_NO_MEMORY
				: QAHIRA_ERROR_CORRUPT_IMAGE,
			Q_("jpeg: %s"), priv->message);
	if (priv->surface) {
		cairo_surface_destroy(priv->surface);
		priv->surface = NULL;
	}
	siglongjmp(priv->env, 1);
}

/**
 * \brief Silence the JPEG library.
 */
static void
output_message(j_common_ptr cinfo)
{
	// do nothing
}

/**
 * \brief JPEG source initialization.
 */
static void
init_source(j_decompress_ptr cinfo)
{
	// do nothing
}

/**
 * \brief Fill the JPEG source buffer.
 *
 * For I/O suspended loading this function must return FALSE.
 */
static gboolean
fill_input_buffer(j_decompress_ptr cinfo)
{
	struct Private *priv = cinfo->client_data;
	if (G_UNLIKELY(!priv->buffer)) {
		g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("input buffer is NULL"));
		siglongjmp(priv->env, 1);
	}
	if (G_UNLIKELY(!priv->input)) {
		goto eoi;
	}
	gssize n = g_input_stream_read(priv->input, priv->buffer,
			priv->size, priv->cancel, priv->error);
	if (-1 == n) {
		siglongjmp(priv->env, 1);
	}
	if (0 == n) {
		goto eoi;
	}
	cinfo->src->bytes_in_buffer = n;
exit:
	cinfo->src->next_input_byte = priv->buffer;
	return TRUE;
eoi:
	priv->buffer[0] = (JOCTET)0xff;
	priv->buffer[1] = (JOCTET)JPEG_EOI;
	cinfo->src->bytes_in_buffer = 2;
	goto exit;
}

/**
 * \brief Skip JPEG input data.
 */
static void
skip_input_data(j_decompress_ptr cinfo, glong bytes)
{
	if (0 < bytes) {
		while (bytes > cinfo->src->bytes_in_buffer) {
			bytes -= cinfo->src->bytes_in_buffer;
			(void)fill_input_buffer(cinfo);
		}
		cinfo->src->next_input_byte += bytes;
		cinfo->src->bytes_in_buffer -= bytes;
	}
}

/**
 * \brief Cleanup JPEG input data.
 */
static void
term_source(j_decompress_ptr cinfo)
{
	// do nothing
}

static void
qahira_image_jpeg_init(QahiraImageJpeg *self)
{
	self->priv = ASSIGN_PRIVATE(self);
	struct Private *priv = GET_PRIVATE(self);
	// initialize error manager
	priv->decompress.err = jpeg_std_error(&priv->error_mgr);
	priv->error_mgr.error_exit = error_exit;
	priv->error_mgr.output_message = output_message;
	// initialize decompress
	GError *error = NULL;
	priv->error = &error;
	if (sigsetjmp(priv->env, 1)) {
		g_message("jpeg: %s", error->message);
		g_error_free(error);
		return;
	}
	jpeg_create_decompress(&priv->decompress);
	priv->decompress.client_data = self->priv;
	// initialize source manager
	priv->decompress.src = &priv->source_mgr;
	priv->source_mgr.init_source = init_source;
	priv->source_mgr.fill_input_buffer = fill_input_buffer;
	priv->source_mgr.skip_input_data = skip_input_data;
	priv->source_mgr.resync_to_restart = jpeg_resync_to_restart;
	priv->source_mgr.term_source = term_source;
	// create input buffer
	priv->size = QAHIRA_JPEG_BUFFER_SIZE;
	priv->buffer = g_try_malloc(priv->size);
	if (G_UNLIKELY(!priv->buffer)) {
		g_message("%s", Q_("jpeg: out of memory"));
	}
}

static void
dispose(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->cancel) {
		g_object_unref(priv->cancel);
		priv->cancel = NULL;
	}
	if (priv->input) {
		g_object_unref(priv->input);
		priv->input = NULL;
	}
	G_OBJECT_CLASS(qahira_image_jpeg_parent_class)->dispose(base);
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	g_free(priv->buffer);
	jpeg_destroy_decompress(&priv->decompress);
	G_OBJECT_CLASS(qahira_image_jpeg_parent_class)->finalize(base);
}

/**
 * \brief Convert a JPEG color space value to a string.
 */
static inline const gchar *
colorspace_name(J_COLOR_SPACE colorspace)
{
	switch (colorspace) {
	case JCS_UNKNOWN:
		return "Unknown"; 
	case JCS_GRAYSCALE:
		return "Grayscale";
	case JCS_RGB:
		return "RGB";
	case JCS_YCbCr:
		return "YCbCr";
	case JCS_CMYK:
		return "CMYK";
	case JCS_YCCK:
		return "YCCK";
	default:
		return "Invalid";
	}
}

/**
 * \brief Convert JPEG grayscale to RGB.
 */
static inline void
convert_grayscale(QahiraImage *self)
{
	struct Private *priv = GET_PRIVATE(self);
	for (gint i = 0; i < priv->decompress.rec_outbuf_height; ++i) {
		guchar *in = priv->lines[i];
		guchar *out = priv->data + priv->stride
			* (i + priv->decompress.output_scanline - 1);
		for (gint j = 0; j < priv->decompress.output_width; ++j) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			out[0] = in[0];
			out[1] = in[0];
			out[2] = in[0];
#else
			out[1] = in[0];
			out[2] = in[0];
			out[3] = in[0];
#endif
			++in;
			out += 4;
		}
	}
}

/**
 * \brief Convert RGB
 */
static inline void
convert_rgb(QahiraImage *self)
{
	struct Private *priv = GET_PRIVATE(self);
	for (gint i = 0; i < priv->decompress.rec_outbuf_height; ++i) {
		guchar *in = priv->lines[i];
		guchar *out = priv->data + priv->stride
			* (i + priv->decompress.output_scanline - 1);
		for (gint j = 0; j < priv->decompress.output_width; ++j) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			out[0] = in[2];
			out[1] = in[1];
			out[2] = in[0];
#else
			out[1] = in[0];
			out[2] = in[1];
			out[3] = in[2];
#endif
			in += 3;
			out += 4;
		}
	}
}

/**
 * \brief Convert JPEG CMYK to RGB.
 */
static inline void
convert_cmyk(QahiraImage *self)
{
	struct Private *priv = GET_PRIVATE(self);
	for (gint i = 0; i < priv->decompress.rec_outbuf_height; ++i) {
		guchar *in = priv->lines[i];
		guchar *out = priv->data + priv->stride
			* (i + priv->decompress.output_scanline - 1);
		for (gint j = 0; j < priv->decompress.output_width; ++j) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			guchar c = in[0];
			guchar m = in[1];
			guchar y = in[2];
			guchar k = in[3];
			if (priv->decompress.saw_Adobe_marker) {
				out[2] = k * c / 255;
				out[1] = k * m / 255;
				out[0] = k * y / 255;
			} else {
				out[2] = (255 - k) * (255 - c) / 255;
				out[1] = (255 - k) * (255 - m) / 255;
				out[0] = (255 - k) * (255 - y) / 255;
			}
#else
			guchar c = in[0];
			guchar m = in[1];
			guchar y = in[2];
			guchar k = in[3];
			if (priv->decompress.saw_Adobe_marker) {
				out[3] = k * c / 255;
				out[2] = k * m / 255;
				out[1] = k * y / 255;
			} else {
				out[3] = (255 - k) * (255 - c) / 255;
				out[2] = (255 - k) * (255 - m) / 255;
				out[1] = (255 - k) * (255 - y) / 255;
			}
#endif
			out += 4;
			in += 4;
		}
	}
}

/**
 * \brief Load JPEG scan lines.
 */
static inline gboolean
load_lines(QahiraImage *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	while (priv->decompress.output_scanline
			< priv->decompress.output_height) {
		gint n = jpeg_read_scanlines(&priv->decompress, priv->lines,
				priv->decompress.rec_outbuf_height);
		if (!n) {
			break;
		}
		switch (priv->decompress.out_color_space) {
		case JCS_GRAYSCALE:
			convert_grayscale(self);
			break;
		case JCS_RGB:
			convert_rgb(self);
			break;
		case JCS_CMYK:
			convert_cmyk(self);
			break;
		default:
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_UNSUPPORTED,
					Q_("jpeg: colorspace %s unsupported"),
					colorspace_name(priv->decompress.out_color_space));
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * \brief Progressively load JPEG scan lines.
 */
static inline gboolean
load_progressive(QahiraImage *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	gboolean in_output = FALSE;
	while (!jpeg_input_complete(&priv->decompress)) {
		if (!in_output) {
			gint scan = priv->decompress.input_scan_number;
			if (jpeg_start_output(&priv->decompress, scan)) {
				in_output = TRUE;
			} else {
				break;
			}
		}
		if (!load_lines(self, error)) {
			return FALSE;
		}
		if (priv->decompress.output_scanline >=
				priv->decompress.output_height
				&& jpeg_finish_output(&priv->decompress)) {
			g_signal_emit(self, signals[SIGNAL_PROGRESSIVE], 0,
					priv->surface);
			in_output = FALSE;
		}
	}
	return TRUE;
}

static cairo_surface_t *
load(QahiraImage *self, GInputStream *stream, GCancellable *cancel,
		GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	if (sigsetjmp(priv->env, 1)) {
		goto error;
	}
	priv->error = error;
	priv->input = g_object_ref(stream);
	if (cancel) {
		priv->cancel = g_object_ref(cancel);
	}
	jpeg_abort_decompress(&priv->decompress);
	jpeg_save_markers(&priv->decompress, JPEG_APP0 + 1, 0xffff);
	jpeg_read_header(&priv->decompress, TRUE);
	jpeg_start_decompress(&priv->decompress);
	priv->surface = qahira_image_surface_create(self, CAIRO_FORMAT_RGB24,
			priv->decompress.output_width,
			priv->decompress.output_height);
	if (!priv->surface) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("jpeg: out of memory"));
		goto exit;
	}
	cairo_status_t err = cairo_surface_status(priv->surface);
	if (CAIRO_STATUS_SUCCESS != err) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_CAIRO,
				"jpeg: %s", cairo_status_to_string(err));
		goto error;
	}
	priv->data = qahira_image_surface_get_data(self, priv->surface);
	if (G_UNLIKELY(!priv->data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: data is NULL"));
		goto error;
	}
	priv->stride = qahira_image_surface_get_stride(self, priv->surface);
	if (G_UNLIKELY(0 > priv->stride)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: invalid stride"));
		goto error;
	}
	if (priv->count != priv->decompress.rec_outbuf_height) {
		priv->lines = priv->decompress.mem->alloc_sarray(
				(j_common_ptr)&priv->decompress,
				JPOOL_IMAGE, priv->stride,
				priv->decompress.rec_outbuf_height);
		if (G_UNLIKELY(!priv->lines)) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_NO_MEMORY,
					Q_("jpeg: out of memory"));
			goto error;
		}
		priv->count = priv->decompress.rec_outbuf_height;
	}
	priv->decompress.buffered_image = priv->decompress.progressive_mode;
	priv->decompress.do_fancy_upsampling = FALSE;
	priv->decompress.do_block_smoothing = FALSE;
	if (priv->decompress.buffered_image) {
		if (!load_progressive(self, error)) {
			goto error;
		}
	} else {
		if (!load_lines(self, error)) {
			goto error;
		}
	}
exit:
	if (priv->input) {
		g_object_unref(priv->input);
		priv->input = NULL;
	}
	if (priv->cancel) {
		g_object_unref(priv->cancel);
		priv->cancel = NULL;
	}
	cairo_surface_t *surface = priv->surface;
	priv->surface = NULL;
	return surface;
error:
	if (priv->surface) {
		cairo_surface_destroy(priv->surface);
		priv->surface = NULL;
	}
	goto exit;
}

static void
qahira_image_jpeg_class_init(QahiraImageJpegClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	QahiraImageClass *image_class = QAHIRA_IMAGE_CLASS(klass);
	image_class->load = load;
	g_type_class_add_private(klass, sizeof(struct Private));
	// QahiraImageJpeg::progressive
	signals[SIGNAL_PROGRESSIVE] =
		g_signal_new(g_intern_static_string("progressive"),
			G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(QahiraImageJpegClass, progressive),
			NULL, NULL, qahira_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, G_TYPE_POINTER);
}

QahiraImage *
qahira_image_jpeg_new(void)
{
	return g_object_new(QAHIRA_TYPE_IMAGE_JPEG,
			"mime-type", "image/jpeg",
			NULL);
}
