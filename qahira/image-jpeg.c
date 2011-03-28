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
	struct jpeg_compress_struct compress;
	struct jpeg_source_mgr source_mgr;
	struct jpeg_destination_mgr destination_mgr;
	struct jpeg_error_mgr error_mgr;
	GInputStream *input;
	GOutputStream *output;
	GCancellable *cancel;
	JOCTET *buffer;
	gint quality;
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
 * \brief Fill the JPEG input buffer.
 */
static gboolean
fill_input_buffer(j_decompress_ptr cinfo)
{
	struct Private *priv = cinfo->client_data;
	if (G_UNLIKELY(!priv->buffer)) {
		g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("jpeg: input buffer is NULL"));
		siglongjmp(priv->env, 1);
	}
	if (G_UNLIKELY(!priv->input)) {
		goto eoi;
	}
	gssize bytes = g_input_stream_read(priv->input, priv->buffer,
			priv->size, priv->cancel, priv->error);
	if (G_UNLIKELY(-1 == bytes)) {
		siglongjmp(priv->env, 1);
	}
	if (G_UNLIKELY(0 == bytes)) {
		goto eoi;
	}
	cinfo->src->bytes_in_buffer = bytes;
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

/**
 * \brief JPEG destination initialization
 */
static void
init_destination(j_compress_ptr cinfo)
{
	struct Private *priv = cinfo->client_data;
	cinfo->dest->next_output_byte = priv->buffer;
	cinfo->dest->free_in_buffer = QAHIRA_JPEG_BUFFER_SIZE;
}

/**
 * \brief Empty the JPEG output buffer;
 */
static boolean
empty_output_buffer(j_compress_ptr cinfo)
{
	struct Private *priv = cinfo->client_data;
	if (G_UNLIKELY(!priv->buffer)) {
		g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("jpeg: input buffer is NULL"));
		siglongjmp(priv->env, 1);
	}
	if (G_UNLIKELY(!priv->output)) {
		g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: output stream is NULL"));
		siglongjmp(priv->env, 1);
	}
	gsize size = QAHIRA_JPEG_BUFFER_SIZE;
	guchar *buffer = priv->buffer;
	while (size) {
		gssize bytes = g_output_stream_write(priv->output, buffer,
				size, priv->cancel, priv->error);
		if (G_UNLIKELY(-1 == bytes)) {
			siglongjmp(priv->env, 1);
		}
		size -= bytes;
		buffer += bytes;
	}
	cinfo->dest->next_output_byte = priv->buffer;
	cinfo->dest->free_in_buffer = QAHIRA_JPEG_BUFFER_SIZE;
	return TRUE;
}

/**
 * \brief Flush data in JPEG output buffer.
 */
static void
term_destination(j_compress_ptr cinfo)
{
	struct Private *priv = cinfo->client_data;
	if (G_UNLIKELY(!priv->buffer)) {
		g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("jpeg: input buffer is NULL"));
		siglongjmp(priv->env, 1);
	}
	if (G_UNLIKELY(!priv->output)) {
		g_set_error(priv->error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: output stream is NULL"));
		siglongjmp(priv->env, 1);
	}
	gsize size = QAHIRA_JPEG_BUFFER_SIZE - cinfo->dest->free_in_buffer;
	guchar *buffer = priv->buffer;
	while (size) {
		gssize bytes = g_output_stream_write(priv->output, buffer,
				size, priv->cancel, priv->error);
		if (G_UNLIKELY(-1 == bytes)) {
			siglongjmp(priv->env, 1);
		}
		size -= bytes;
		buffer += bytes;
	}
}

static void
qahira_image_jpeg_init(QahiraImageJpeg *self)
{
	self->priv = ASSIGN_PRIVATE(self);
	struct Private *priv = GET_PRIVATE(self);
	// initialize error manager
	priv->decompress.err = priv->compress.err =
		jpeg_std_error(&priv->error_mgr);
	priv->error_mgr.error_exit = error_exit;
	priv->error_mgr.output_message = output_message;
	GError *error = NULL;
	priv->error = &error;
	if (sigsetjmp(priv->env, 1)) {
		g_message("jpeg: %s", error->message);
		g_error_free(error);
		return;
	}
	// initialize decompress
	jpeg_create_decompress(&priv->decompress);
	priv->decompress.client_data = priv;
	// initialize compress
	jpeg_create_compress(&priv->compress);
	priv->compress.client_data = priv;
	priv->quality = 75;
	// initialize source manager
	priv->decompress.src = &priv->source_mgr;
	priv->source_mgr.init_source = init_source;
	priv->source_mgr.fill_input_buffer = fill_input_buffer;
	priv->source_mgr.skip_input_data = skip_input_data;
	priv->source_mgr.resync_to_restart = jpeg_resync_to_restart;
	priv->source_mgr.term_source = term_source;
	// initialize destination manager
	priv->compress.dest = &priv->destination_mgr;
	priv->destination_mgr.init_destination = init_destination;
	priv->destination_mgr.empty_output_buffer = empty_output_buffer;
	priv->destination_mgr.term_destination = term_destination;
	// initialize I/O buffer
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
	jpeg_destroy_compress(&priv->compress);
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
			out[QAHIRA_R] = in[0];
			out[QAHIRA_G] = in[0];
			out[QAHIRA_B] = in[0];
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
			out[QAHIRA_R] = in[2];
			out[QAHIRA_G] = in[1];
			out[QAHIRA_B] = in[0];
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
			guchar c = in[0];
			guchar m = in[1];
			guchar y = in[2];
			guchar k = in[3];
			if (priv->decompress.saw_Adobe_marker) {
				out[QAHIRA_R] = k * y / 255;
				out[QAHIRA_G] = k * m / 255;
				out[QAHIRA_B] = k * c / 255;
			} else {
				out[QAHIRA_R] = (255 - k) * (255 - y) / 255;
				out[QAHIRA_G] = (255 - k) * (255 - m) / 255;
				out[QAHIRA_B] = (255 - k) * (255 - c) / 255;
			}
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
				Q_("jpeg: surface data is NULL"));
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
	jpeg_finish_decompress(&priv->decompress);
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

static gboolean
save(QahiraImage *self, cairo_surface_t *surface, GOutputStream *stream,
		GCancellable *cancel, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	cairo_content_t content = cairo_surface_get_content(surface);
	gboolean status = TRUE;
	guchar *buffer = NULL;
	gint width, height;
	qahira_image_surface_size(surface, &width, &height);
	if (!width || !height) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: invalid dimensions [%d x %d]"),
				width, height);
		goto error;
	}
	priv->error = error;
	if (sigsetjmp(priv->env, 1)) {
		goto error;
	}
	priv->output = g_object_ref(stream);
	if (cancel) {
		priv->cancel = g_object_ref(cancel);
	}
	guchar *data = qahira_image_surface_get_data(self, surface);
	if (G_UNLIKELY(!data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: surface data is NULL"));
		goto error;
	}
	gint stride = 0;
	gint components;
	gint color_space;
	switch (content) {
	case CAIRO_CONTENT_COLOR:
	case CAIRO_CONTENT_COLOR_ALPHA:
		components = 3;
		color_space = JCS_RGB;
		buffer = g_try_malloc(components * width);
		if (!buffer) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_NO_MEMORY,
					Q_("jpeg: out of memory"));
			goto error;
		}
		break;
	case CAIRO_CONTENT_ALPHA:
		components = 1;
		color_space = JCS_GRAYSCALE;
		stride = qahira_image_surface_get_stride(self, surface);
		if (0 > stride) {
			g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
					Q_("jpeg: invalid stride"));
			goto error;
		}
		break;
	default:
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("jpeg: unsupported surface content"));
		goto error;
	}
	jpeg_abort_compress(&priv->compress);
	priv->compress.image_width = width;
	priv->compress.image_height = height;
	priv->compress.input_components = components;
	priv->compress.in_color_space = color_space;
	jpeg_set_defaults(&priv->compress);
	jpeg_set_quality(&priv->compress, priv->quality, TRUE);
	jpeg_start_compress(&priv->compress, TRUE);
	gint i = 0;
	while (priv->compress.next_scanline < priv->compress.image_height) {
		switch (content) {
		case CAIRO_CONTENT_COLOR:
		case CAIRO_CONTENT_COLOR_ALPHA:
			for (gint j = 0; j < width; ++j) {
				buffer[3 * j + 0] = data[QAHIRA_R];
				buffer[3 * j + 1] = data[QAHIRA_G];
				buffer[3 * j + 2] = data[QAHIRA_B];
				data += 4;
			}
			break;
		case CAIRO_CONTENT_ALPHA:
			buffer = data + i * stride;
			break;
		default:
			g_assert_not_reached();
		}
		jpeg_write_scanlines(&priv->compress, &buffer, 1);
		++i;
	}
	jpeg_finish_compress(&priv->compress);
exit:
	switch (content) {
	case CAIRO_CONTENT_COLOR:
	case CAIRO_CONTENT_COLOR_ALPHA:
		g_free(buffer);
		break;
	case CAIRO_CONTENT_ALPHA:
	default:
		break;
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
qahira_image_jpeg_class_init(QahiraImageJpegClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	QahiraImageClass *image_class = QAHIRA_IMAGE_CLASS(klass);
	image_class->load = load;
	image_class->save = save;
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

void
qahira_image_jpeg_set_quality(QahiraImage *self, gint quality)
{
	g_return_if_fail(QAHIRA_IS_IMAGE_JPEG(self));
	GET_PRIVATE(self)->quality = CLAMP(quality, 0, 100);
}

gint
qahira_image_jpeg_get_quality(QahiraImage *self)
{
	g_return_val_if_fail(QAHIRA_IS_IMAGE_JPEG(self), 0);
	return GET_PRIVATE(self)->quality;
}
