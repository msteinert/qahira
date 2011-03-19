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
#include <gio/gunixinputstream.h>
#include "qahira/loader/factory.h"
#include "qahira/loader/private.h"
#include "qahira/macros.h"
#include "qahira/qahira.h"
#include "qahira/surface/factory/image.h"
#include "qahira/surface/factory.h"

G_DEFINE_TYPE(Qahira, qahira, G_TYPE_OBJECT)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_QAHIRA, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((Qahira *)instance)->priv)

struct Private {
	GPtrArray *loaders;
	QahiraLoaderFactory *loader_factory;
	QahiraSurfaceFactory *surface_factory;
	GInputStream *stream;
	GCancellable *cancel;
	gchar *filename;
	gboolean owner;
	guchar buffer[4096];
	gsize size;
};

static void
destroy(gpointer object)
{
	g_object_run_dispose(G_OBJECT(object));
	g_object_unref(object);
}

static void
qahira_init(Qahira *self)
{
	self->priv = ASSIGN_PRIVATE(self);
	struct Private *priv = GET_PRIVATE(self);
	priv->loaders = g_ptr_array_new_with_free_func(destroy);
	priv->loader_factory = qahira_loader_factory_new();
	priv->surface_factory = qahira_image_surface_factory_new();
}

static void
dispose(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->loaders) {
		g_ptr_array_free(priv->loaders, TRUE);
		priv->loaders = NULL;
	}
	if (priv->loader_factory) {
		g_object_unref(priv->loader_factory);
		priv->loader_factory = NULL;
	}
	if (priv->surface_factory) {
		g_object_unref(priv->surface_factory);
		priv->surface_factory = NULL;
	}
	if (priv->stream) {
		g_object_unref(priv->stream);
		priv->stream = NULL;
	}
	if (priv->cancel) {
		g_object_unref(priv->cancel);
		priv->cancel = NULL;
	}
	G_OBJECT_CLASS(qahira_parent_class)->dispose(base);
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->filename && priv->owner) {
		g_free(priv->filename);
	}
	G_OBJECT_CLASS(qahira_parent_class)->finalize(base);
}

static void
qahira_class_init(QahiraClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	g_type_class_add_private(klass, sizeof(struct Private));
}

Qahira *
qahira_new(void)
{
	return g_object_new(QAHIRA_TYPE_QAHIRA, NULL);
}

static void
reset(Qahira *self)
{
	g_return_if_fail(QAHIRA_IS_QAHIRA(self));
	struct Private *priv = GET_PRIVATE(self);
	if (priv->stream) {
		g_object_unref(priv->stream);
		priv->stream = NULL;
	}
	if (priv->filename) {
		if (priv->owner) {
			g_free(priv->filename);
		}
		priv->filename = NULL;
	}
}

cairo_surface_t *
qahira_surface_create(Qahira *self, GError **error)
{
	g_return_val_if_fail(QAHIRA_IS_QAHIRA(self), NULL);
	struct Private *priv = GET_PRIVATE(self);
	cairo_surface_t *surface = NULL;
	gchar *type = NULL;
	if (!priv->stream) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("image source is not set"));
		goto exit;
	}
	gsize size = g_input_stream_read(priv->stream, priv->buffer,
			sizeof(priv->buffer), priv->cancel, error);
	if (-1 == size) {
		goto exit;
	}
	if (0 == size) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_EMPTY_FILE,
				Q_("stream is empty"));
		goto exit;
	}
	type = g_content_type_guess(priv->filename, priv->buffer, size, NULL);
	if (G_UNLIKELY(!type)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("failed to guess content type"));
		goto exit;
	}
	QahiraLoader *loader = qahira_get_loader(self, type);
	if (G_UNLIKELY(!loader)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
				Q_("unsupported content type `%s'"), type);
		goto exit;
	}
	qahira_loader_set_surface_factory(loader, priv->surface_factory);
	gboolean status = qahira_loader_load_start(loader, error);
	if (!status) {
		goto exit;
	}
	gssize read = 0;
	while (TRUE) {
		gsize remaining = size - read;
		if (remaining == 0) {
			size = g_input_stream_read(priv->stream, priv->buffer,
					sizeof(priv->buffer), priv->cancel,
					error);
			if (-1 == size) {
				goto error;
			}
			if (0 == size) {
				break;
			}
			remaining = size;
			read = 0;
		}
		read = qahira_loader_load_increment(loader,
				priv->buffer + read, remaining, error);
		if (-1 == read) {
			goto error;
		}
	}
	surface = qahira_loader_load_finish(loader);
exit:
	g_free(type);
	return surface;
error:
	surface = qahira_loader_load_finish(loader);
	if (surface) {
		cairo_surface_destroy(surface);
	}
	surface = NULL;
	goto exit;
}

static inline gboolean
open_file_stream(Qahira *self, const gchar *filename, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	GFile *file = g_file_new_for_path(filename);
	if (G_UNLIKELY(!file)) {
		g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE,
				Q_("%s: g_file_new_for_path returned NULL"),
				filename);
		return FALSE;
	}
	priv->stream = (GInputStream *)g_file_read(file, priv->cancel, error);
	g_object_unref(file);
	return priv->stream ? TRUE : FALSE;
}

gboolean
qahira_set_filename(Qahira *self, const gchar *filename, GError **error)
{
	reset(self);
	qahira_return_error_if_fail(QAHIRA_IS_QAHIRA(self), FALSE, error);
	qahira_return_error_if_fail(filename, FALSE, error);
	struct Private *priv = GET_PRIVATE(self);
	priv->filename = g_strdup(filename);
	priv->owner = TRUE;
	return open_file_stream(self, filename, error);
}

gboolean
qahira_set_static_filename(Qahira *self, const gchar *filename,
		GError **error)
{
	reset(self);
	qahira_return_error_if_fail(QAHIRA_IS_QAHIRA(self), FALSE, error);
	qahira_return_error_if_fail(filename, FALSE, error);
	struct Private *priv = GET_PRIVATE(self);
	priv->filename = (gchar *)filename;
	priv->owner = FALSE;
	return open_file_stream(self, filename, error);
}

void
qahira_set_descriptor(Qahira *self, int descriptor)
{
	reset(self);
	g_return_if_fail(QAHIRA_IS_QAHIRA(self));
	g_return_if_fail(-1 < descriptor);
	struct Private *priv = GET_PRIVATE(self);
	priv->stream = g_unix_input_stream_new(descriptor, FALSE);
}

void
qahira_set_file(Qahira *self, FILE *file)
{
	reset(self);
	g_return_if_fail(QAHIRA_IS_QAHIRA(self));
	g_return_if_fail(file);
	struct Private *priv = GET_PRIVATE(self);
	int descriptor = fileno(file);
	priv->stream = g_unix_input_stream_new(descriptor, FALSE);
}

void
qahira_set_stream(Qahira *self, GInputStream *stream)
{
	reset(self);
	g_return_if_fail(QAHIRA_IS_QAHIRA(self));
	g_return_if_fail(G_IS_INPUT_STREAM(stream));
	struct Private *priv = GET_PRIVATE(self);
	priv->stream = g_object_ref(stream);
}

void
qahira_set_cancellable(Qahira *self, GCancellable *cancel)
{
	g_return_if_fail(QAHIRA_IS_QAHIRA(self));
	struct Private *priv = GET_PRIVATE(self);
	if (priv->cancel) {
		g_object_unref(priv->cancel);
	}
	if (cancel && G_IS_CANCELLABLE(cancel)) {
		priv->cancel = g_object_ref(cancel);
	} else {
		priv->cancel = NULL;
	}
}

GCancellable *
qahira_get_cancellable(Qahira *self)
{
	g_return_val_if_fail(QAHIRA_IS_QAHIRA(self), NULL);
	return GET_PRIVATE(self)->cancel;
}

QahiraLoaderFactory *
qahira_get_loader_factory(Qahira *self)
{
	g_return_val_if_fail(QAHIRA_IS_QAHIRA(self), NULL);
	return GET_PRIVATE(self)->loader_factory;
}

void
qahira_set_surface_factory(Qahira *self, QahiraSurfaceFactory *factory)
{
	g_return_if_fail(QAHIRA_IS_QAHIRA(self));
	g_return_if_fail(QAHIRA_IS_SURFACE_FACTORY(factory));
	struct Private *priv = GET_PRIVATE(self);
	if (priv->surface_factory) {
		g_object_unref(priv->surface_factory);
	}
	priv->surface_factory = g_object_ref(factory);
}

QahiraSurfaceFactory *
qahira_get_surface_factory(Qahira *self)
{
	g_return_val_if_fail(QAHIRA_IS_QAHIRA(self), NULL);
	return GET_PRIVATE(self)->surface_factory;
}

QahiraLoader *
qahira_get_loader(Qahira *self, const gchar *type)
{
	g_return_val_if_fail(QAHIRA_IS_QAHIRA(self), NULL);
	struct Private *priv = GET_PRIVATE(self);
	if (G_UNLIKELY(!priv->loaders)) {
		return NULL;
	}
	const gchar *string = g_intern_string(type);
	if (G_UNLIKELY(!string)) {
		return NULL;
	}
	for (gint i = 0; i < priv->loaders->len; ++i) {
		QahiraLoader *loader = priv->loaders->pdata[i];
		if (qahira_loader_supports_intern_string(loader, string)) {
			return loader;
		}
	}
	if (G_UNLIKELY(!priv->loader_factory)) {
		return NULL;
	}
	QahiraLoader *loader =
		qahira_loader_factory_create(priv->loader_factory, type);
	if (loader) {
		g_ptr_array_add(priv->loaders, loader);
		return loader;
	}
	return NULL;
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
