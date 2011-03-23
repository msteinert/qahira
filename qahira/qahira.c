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
#if QAHIRA_HAS_JPEG
#include "qahira/image/jpeg.h"
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
#include "qahira/image/png.h"
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
#include "qahira/image/targa.h"
#endif // QAHIRA_HAS_TARGA
#include "qahira/macros.h"
#include "qahira/marshal.h"
#include "qahira/qahira.h"

G_DEFINE_TYPE(Qahira, qahira, G_TYPE_OBJECT)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_QAHIRA, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((Qahira *)instance)->priv)

enum Signals {
	SIGNAL_GET_IMAGE,
	SIGNAL_LAST
};

static guint signals[SIGNAL_LAST] = { 0 };

#define QAHIRA_BUFFER_SIZE (1024 * 4)

struct Private {
	GSList *formats;
};

static void
qahira_init(Qahira *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
dispose(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	for (GSList *node = priv->formats; node; node = node->next) {
		QahiraImage *image = node->data;
		g_object_unref(image);
	}
	G_OBJECT_CLASS(qahira_parent_class)->dispose(base);
}

static QahiraImage *
get_image(Qahira *self, const gchar *mime)
{
#if QAHIRA_HAS_JPEG
	if (g_content_type_equals(mime, "image/jpeg")) {
		return qahira_image_jpeg_new();
	}
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
	if (g_content_type_equals(mime, "image/png")) {
		return qahira_image_png_new();
	}
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
	if (g_content_type_equals(mime, "image/x-tga")) {
		return qahira_image_targa_new();
	}
#endif // QAHIRA_HAS_TARGA
	return NULL;
}

static void
qahira_class_init(QahiraClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	klass->get_image = get_image;
	g_type_class_add_private(klass, sizeof(struct Private));
	// JoyBubble::hide
	signals[SIGNAL_GET_IMAGE] =
		g_signal_new(g_intern_static_string("get-image"),
			G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(QahiraClass, get_image),
			qahira_object_accumulator, NULL,
			qahira_marshal_OBJECT__STRING, G_TYPE_OBJECT,
			1, G_TYPE_STRING);
}

Qahira *
qahira_new(void)
{
	return g_object_new(QAHIRA_TYPE_QAHIRA, NULL);
}

cairo_surface_t *
qahira_load_file(Qahira *self, GFile *file, GCancellable *cancel,
		GError **error)
{
	qahira_return_error_if_fail(QAHIRA_IS_QAHIRA(self), NULL, error);
	qahira_return_error_if_fail(G_IS_FILE(file), NULL, error);
	cairo_surface_t *surface = NULL;
	GInputStream *stream = NULL;
	guchar *buffer = NULL;
	gchar *mime = NULL;
	gchar *filename = g_file_get_path(file);
	if (G_UNLIKELY(!filename)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_NO_MEMORY,
				Q_("out of memory"));
		goto exit;
	}
	stream = G_INPUT_STREAM(g_file_read(file, cancel, error));
	if (G_UNLIKELY(!stream)) {
		goto exit;
	}
	if (G_IS_SEEKABLE(stream)
			&& g_seekable_can_seek(G_SEEKABLE(stream))) {
		buffer = g_try_malloc(QAHIRA_BUFFER_SIZE);
		if (G_UNLIKELY(!buffer)) {
			g_set_error(error, QAHIRA_ERROR,
					QAHIRA_ERROR_NO_MEMORY,
					Q_("out of memory"));
			goto exit;
		}
		gssize size = g_input_stream_read(stream, buffer,
				QAHIRA_BUFFER_SIZE, cancel, error);
		if (-1 == size) {
			goto exit;
		}
		if (!g_seekable_seek(G_SEEKABLE(stream), 0, G_SEEK_SET,
				cancel, error)) {
			goto exit;
		}
	}
	mime = g_content_type_guess(filename, buffer,
			buffer ? QAHIRA_BUFFER_SIZE : 0, NULL);
	if (G_UNLIKELY(!mime)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("failed to mime content type"));
		goto exit;
	}
	QahiraImage *image = qahira_get_image(self, mime);
	if (G_UNLIKELY(!image)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("unsupported mime type `%s'"), mime);
		goto exit;
	}
	surface = qahira_image_load(image, stream, cancel, error);
exit:
	g_free(filename);
	g_free(buffer);
	g_free(mime);
	if (stream) {
		g_object_unref(stream);
	}
	return surface;
}

cairo_surface_t *
qahira_load_filename(Qahira *self, const gchar *filename,
		GCancellable *cancel, GError **error)
{
	qahira_return_error_if_fail(QAHIRA_IS_QAHIRA(self), NULL, error);
	qahira_return_error_if_fail(filename, NULL, error);
	cairo_surface_t *surface = NULL;
	GFile *file = g_file_new_for_path(filename);
	if (!file) {
		goto exit;
	}
	surface = qahira_load_file(self, file, cancel, error);
exit:
	if (file) {
		g_object_unref(file);
	}
	return surface;
}

QahiraImage *
qahira_get_image(Qahira *self, const gchar *mime)
{
	g_return_val_if_fail(QAHIRA_IS_QAHIRA(self), NULL);
	g_return_val_if_fail(mime, NULL);
	struct Private *priv = GET_PRIVATE(self);
	const gchar *string = g_intern_string(mime);
	if (G_UNLIKELY(!string)) {
		return NULL;
	}
	QahiraImage *image = NULL;
	for (GSList *node = priv->formats; node; node = node->next) {
		image = node->data;
		if (qahira_image_supports_intern_string(image, string)) {
			return image;
		}
	}
	g_signal_emit(self, signals[SIGNAL_GET_IMAGE], 0, mime, &image);
	if (image) {
		priv->formats = g_slist_prepend(priv->formats, image);
	}
	return image;
}

#ifdef QAHIRA_TRACE
#ifdef HAVE_BFD
#include <bfd.h>
#else // HAVE_BFD
#ifdef HAVE_DLADDR
#include <dlfcn.h>
#endif // HAVE_DLADDR
#endif // HAVE_BFD
G_GNUC_NO_INSTRUMENT
void
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
#ifdef HAVE_BFD
	g_print(Q_("[TRACE] enter [%p] (TODO libbfd tracing)\n"), this_fn);
#else // HAVE_BFD
#ifdef HAVE_DLADDR
	Dl_info info;
	if (dladdr(this_fn, &info) && info.dli_sname) {
		g_print(Q_("[TRACE] enter %s [%p]\n"), info.dli_sname,
				this_fn);
	} else {
		g_print(Q_("[TRACE] enter [%p]\n"), this_fn);
	}
#else // HAVE_DLADDR
	g_print(Q_("[TRACE] enter [%p]\n"), this_fn);
#endif // HAVE_DLADDR
#endif // HAVE_BFD
}

G_GNUC_NO_INSTRUMENT
void
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
#ifdef HAVE_BFD
	g_print(Q_("[TRACE] exit [%p] (TODO libbfd tracing)\n"), this_fn);
#else // HAVE_BFD
#ifdef HAVE_DLADDR
	Dl_info info;
	if (dladdr(this_fn, &info) && info.dli_sname) {
		g_print(Q_("[TRACE] exit %s [%p]\n"), info.dli_sname,
				this_fn);
	} else {
		g_print(Q_("[TRACE] exit [%p]\n"), this_fn);
	}
#else // HAVE_DLADDR
	g_print(Q_("[TRACE] exit [%p]\n"), this_fn);
#endif // HAVE_DLADDR
#endif // HAVE_BFD
}
#endif // QAHIRA_TRACE
