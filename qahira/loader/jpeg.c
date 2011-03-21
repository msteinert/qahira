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
#include "qahira/loader/jpeg.h"
#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <jerror.h>

G_DEFINE_TYPE(QahiraLoaderJpeg, qahira_loader_jpeg, QAHIRA_TYPE_LOADER)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_LOADER_JPEG, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraLoaderJpeg *)instance)->priv)

struct Private {
	struct jpeg_decompress_struct decompress;
	struct jpeg_source_mgr source_mgr;
	struct jpeg_error_mgr error_mgr;
	glong skip;
	gint stride;
	guchar *data;
	sigjmp_buf env;
	GError **error;
	gboolean in_output;
	gboolean got_header;
	cairo_surface_t *surface;
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
	return FALSE;
}

/**
 * \brief Skip JPEG input data.
 */
static void
skip_input_data(j_decompress_ptr cinfo, glong bytes)
{
	if (0 < bytes) {
		struct Private *priv = cinfo->client_data;
		glong skip = MIN(cinfo->src->bytes_in_buffer, bytes);
		cinfo->src->next_input_byte += skip;
		cinfo->src->bytes_in_buffer -= skip;
		priv->skip = bytes - skip;
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
qahira_loader_jpeg_init(QahiraLoaderJpeg *self)
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
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	jpeg_destroy_decompress(&priv->decompress);
	if (priv->surface) {
		cairo_surface_destroy(priv->surface);
	}
	G_OBJECT_CLASS(qahira_loader_jpeg_parent_class)->finalize(base);
}

static gboolean
load_start(QahiraLoader *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	priv->error = error;
	if (sigsetjmp(priv->env, 1)) {
		return FALSE;
	}
	if (priv->surface) {
		cairo_surface_destroy(priv->surface);
		priv->surface = NULL;
	}
	priv->skip = 0;
	priv->in_output = FALSE;
	priv->got_header = FALSE;
	jpeg_abort_decompress(&priv->decompress);
	return TRUE;
}

/**
 * \brief Convert a JPEG color space value to a string.
 */
static inline const gchar *
colorspace_name(J_COLOR_SPACE colorspace)
{
	switch (colorspace) {
	case JCS_UNKNOWN:
		return "UNKNOWN"; 
	case JCS_GRAYSCALE:
		return "GRAYSCALE";
	case JCS_RGB:
		return "RGB";
	case JCS_YCbCr:
		return "YCbCr";
	case JCS_CMYK:
		return "CMYK";
	case JCS_YCCK:
		return "YCCK";
	default:
		return "invalid";
	}
}

/**
 * \brief Convert JPEG grayscale to RGB.
 */
static inline void
convert_grayscale(QahiraLoader *self, guchar **lines)
{
	struct Private *priv = GET_PRIVATE(self);
	gint w = priv->decompress.output_width;
	for (gint i = priv->decompress.rec_outbuf_height - 1; i >= 0; --i) {
		guchar *to, *from;
		to = lines[i] + (w - 1) * 3;
		from = lines[i] + w - 1;
		for (gint j = w - 1; j >= 0; --j) {
			to[0] = from[0];
			to[1] = from[0];
			to[2] = from[0];
			to -= 3;
			--from;
		}
	}
}

/**
 * \brief Convert JPEG CMYK to RGB.
 */
static inline void
convert_cmyk(QahiraLoader *self, guchar **lines)
{
	struct Private *priv = GET_PRIVATE(self);
	for (gint i = priv->decompress.rec_outbuf_height - 1; i >= 0; --i) {
		guchar *p = lines[i];
		for (gint j = 0; j < priv->decompress.output_width; ++j) {
			gint c = p[0];
			gint m = p[1];
			gint y = p[2];
			gint k = p[3];
			if (priv->decompress.saw_Adobe_marker) {
				p[0] = k * c / 255;
				p[1] = k * m / 255;
				p[2] = k * y / 255;
			} else {
				p[0] = (255 - k) * (255 - c) / 255;
				p[1] = (255 - k) * (255 - m) / 255;
				p[2] = (255 - k) * (255 - y) / 255;
			}
			p[3] = 255;
			p += 4;
		}
	}
}

/**
 * \brief Load the JPEG image header.
 */
static inline gboolean
load_header(QahiraLoader *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	gint status;
	if (!priv->got_header) {
		status = jpeg_read_header(&priv->decompress, TRUE);
		if (JPEG_SUSPENDED == status) {
			return TRUE;
		}
		priv->got_header = TRUE;
	}
	status = jpeg_start_decompress(&priv->decompress);
	if (JPEG_SUSPENDED == status) {
		return TRUE;
	}
	priv->decompress.buffered_image = priv->decompress.progressive_mode;
	priv->decompress.do_fancy_upsampling = FALSE;
	priv->decompress.do_block_smoothing = FALSE;
	priv->surface = qahira_loader_surface_create(self,
			priv->decompress.output_components == 4
				? CAIRO_FORMAT_ARGB32
				: CAIRO_FORMAT_RGB24,
			priv->decompress.output_width,
			priv->decompress.output_height);
	if (G_UNLIKELY(!priv->surface)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("jpeg: failed to create surface"));
		return FALSE;
	}
	cairo_status_t err = cairo_surface_status(priv->surface);
	if (G_UNLIKELY(CAIRO_STATUS_SUCCESS != err)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: %s"), cairo_status_to_string(err));
		return FALSE;
	}
	cairo_surface_flush(priv->surface);
	priv->data = qahira_loader_surface_get_data(self, priv->surface);
	if (G_UNLIKELY(!priv->data)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: surface data is NULL"));
		return FALSE;
	}
	priv->stride = qahira_loader_surface_get_stride(self, priv->surface);
	if (G_UNLIKELY(!priv->stride)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("jpeg: stride is zero"));
		return FALSE;
	}
	return TRUE;
}

/**
 * \brief Load JPEG scan lines.
 */
static inline gboolean
load_lines(QahiraLoader *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	guchar *lines[4], **lptr, *rptr;
	while (priv->decompress.output_scanline
			< priv->decompress.output_height) {
		lptr = lines;
		rptr = priv->data;
		for (gint i = 0; i < priv->decompress.rec_outbuf_height;
				++i) {
			*lptr++ = rptr;
			rptr += priv->stride;
		}
		gint n = jpeg_read_scanlines(&priv->decompress, lines,
				priv->decompress.rec_outbuf_height);
		if (!n) {
			break;
		}
		switch (priv->decompress.out_color_space) {
		case JCS_GRAYSCALE:
			convert_grayscale(self, lines);
			break;
		case JCS_RGB:
			// nothing to do
			break;
		case JCS_CMYK:
			convert_cmyk(self, lines);
			break;
		default:
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_UNSUPPORTED,
					Q_("jpeg: colorspace %s unsupported"),
					colorspace_name(priv->decompress.out_color_space));
			return FALSE;
		}
		priv->data += n * priv->stride;
	}
	return TRUE;
}

/**
 * \brief Progressively load JPEG scan lines.
 */
static inline gboolean
load_progressive(QahiraLoader *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	while (!jpeg_input_complete(&priv->decompress)) {
		if (!priv->in_output) {
			gint scan = priv->decompress.input_scan_number;
			if (jpeg_start_output(&priv->decompress, scan)) {
				priv->in_output = TRUE;
				priv->data = qahira_loader_surface_get_data(
						self, priv->surface);
				if (G_UNLIKELY(!priv->data)) {
					g_set_error(error, QAHIRA_ERROR,
						QAHIRA_ERROR_FAILURE,
						Q_("jpeg: data is NULL"));
					return FALSE;
				}
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
			priv->in_output = FALSE;
		}
	}
	return TRUE;
}

static gssize
load_increment(QahiraLoader *self, guchar *buffer, gsize size, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	// set input buffer
	if (priv->skip) {
		if (priv->skip >= size) {
			priv->skip -= size;
			return size;
		} else {
			priv->source_mgr.next_input_byte = buffer + priv->skip;
			priv->source_mgr.bytes_in_buffer = size - priv->skip;
			priv->skip = 0;
		}
	} else {
		priv->source_mgr.next_input_byte = buffer;
		priv->source_mgr.bytes_in_buffer = size;
	}
	// set error handler
	priv->error = error;
	if (sigsetjmp(priv->env, 1)) {
		return -1;
	}
	// parse image data
	if (!priv->surface) {
		if (!load_header(self, error)) {
			return -1;
		}
	} else {
		if (priv->decompress.buffered_image) {
			if (!load_progressive(self, error)) {
				return -1;
			}
		} else {
			if (!load_lines(self, error)) {
				return -1;
			}
		}
	}
	return size;
}

static cairo_surface_t *
load_finish(QahiraLoader *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	priv->error = error;
	if (sigsetjmp(priv->env, 1)) {
		return NULL;
	}
	jpeg_finish_decompress(&priv->decompress);
	cairo_surface_t *surface = NULL;
	if (priv->surface) {
		surface = priv->surface;
		priv->surface = NULL;
		cairo_surface_mark_dirty(surface);
	}
	return surface;
}

#if 0
static gboolean
save_start(QahiraLoader *self, GError **error)
{
	// TODO
	return FALSE;
}

static gboolean        
save(QahiraLoader *self, cairo_surface_t *surface, GOutputStream *stream,
		GCancellable *cancel, GError **error)
{
	// TODO
	return FALSE;
}

static void
save_finish(QahiraLoader *self)
{
	// TODO
}
#endif

static void
qahira_loader_jpeg_class_init(QahiraLoaderJpegClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = finalize;
	QahiraLoaderClass *loader_class = QAHIRA_LOADER_CLASS(klass);
	loader_class->load_start = load_start;
	loader_class->load_increment = load_increment;
	loader_class->load_finish = load_finish;
#if 0
	loader_class->save_start = save_start;
	loader_class->save = save;
	loader_class->save_finish = save_finish;
#endif
	g_type_class_add_private(klass, sizeof(struct Private));
}

QahiraLoader *
qahira_loader_jpeg_new(void)
{
	return g_object_new(QAHIRA_TYPE_LOADER_JPEG, NULL);
}
