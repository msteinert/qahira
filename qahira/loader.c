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
#include "qahira/loader/private.h"
#include "qahira/macros.h"
#include "qahira/surface/factory.h"

G_DEFINE_ABSTRACT_TYPE(QahiraLoader, qahira_loader, G_TYPE_OBJECT)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_LOADER, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraLoader *)instance)->priv)

struct Private {
	QahiraSurfaceFactory *factory;
	GSList *types;
	guchar buffer[4096];
};

static void
qahira_loader_init(QahiraLoader *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
dispose(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->factory) {
		g_object_unref(priv->factory);
		priv->factory = NULL;
	}
	G_OBJECT_CLASS(qahira_loader_parent_class)->dispose(base);
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->types) {
		g_slist_free(priv->types);
	}
	G_OBJECT_CLASS(qahira_loader_parent_class)->finalize(base);
}

static gboolean
load_start(QahiraLoader *self, GError **error)
{
	g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
			Q_("load operation is unsupported"));
	return FALSE;
}

static gboolean
save_start(QahiraLoader *self, GError **error)
{
	g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_UNSUPPORTED,
			Q_("save operation is unsupported"));
	return FALSE;
}

static void
qahira_loader_class_init(QahiraLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	klass->load_start = load_start;
	klass->save_start = save_start;
	g_type_class_add_private(klass, sizeof(struct Private));
}

void
qahira_loader_set_surface_factory(QahiraLoader *self,
		QahiraSurfaceFactory *factory)
{
	g_return_if_fail(QAHIRA_IS_LOADER(self));
	g_return_if_fail(QAHIRA_IS_SURFACE_FACTORY(factory));
	struct Private *priv = GET_PRIVATE(self);
	if (priv->factory) {
		g_object_unref(priv->factory);
	}
	priv->factory = g_object_ref(factory);
}

QahiraSurfaceFactory *
qahira_loader_get_surface_factory(QahiraLoader *self)
{
	g_return_val_if_fail(QAHIRA_IS_LOADER(self), NULL);
	return GET_PRIVATE(self)->factory;
}

cairo_surface_t *
qahira_loader_load(QahiraLoader *self, GInputStream *stream,
		GCancellable *cancel, GError **error)
{
	qahira_return_error_if_fail(QAHIRA_IS_LOADER(self), NULL, error);
	qahira_return_error_if_fail(G_IS_INPUT_STREAM(stream), NULL, error);
	if (cancel) {
		qahira_return_error_if_fail(G_IS_CANCELLABLE(cancel), FALSE,
				error);
	}
	struct Private *priv = GET_PRIVATE(self);
	gboolean status = qahira_loader_load_start(self, error);
	if (!status) {
		return NULL;
	}
	gsize size = 0;
	gssize read = 0;
	while (TRUE) {
		gsize remaining = size - read;
		if (0 == remaining) {
			size = g_input_stream_read(stream, priv->buffer,
					sizeof(priv->buffer), cancel, error);
			if (-1 == size) {
				return NULL;
			}
			if (0 == size) {
				break;
			}
			remaining = size;
			read = 0;
		}
		read = qahira_loader_load_increment(self, priv->buffer + read,
				remaining, error);
		if (-1 == read) {
			return NULL;
		}
	}
	return qahira_loader_load_finish(self, error);
}

gboolean
qahira_loader_save(QahiraLoader *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error)
{
	qahira_return_error_if_fail(QAHIRA_IS_LOADER(self), FALSE, error);
	qahira_return_error_if_fail(surface, FALSE, error);
	qahira_return_error_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE, error);
	if (cancel) {
		qahira_return_error_if_fail(G_IS_CANCELLABLE(cancel), FALSE,
				error);
	}
	gboolean status = QAHIRA_LOADER_GET_CLASS(self)->
		save_start(self, error);
	if (!status) {
		return FALSE;
	}
	status = QAHIRA_LOADER_GET_CLASS(self)->
		save(self, surface, stream, cancel, error);
	QAHIRA_LOADER_GET_CLASS(self)->save_finish(self);
	return status;
}

gboolean
qahira_loader_supports(QahiraLoader *self, const gchar *type)
{
	g_return_val_if_fail(QAHIRA_IS_LOADER(self), FALSE);
	g_return_val_if_fail(type, FALSE);
	const gchar *string = g_intern_string(type);
	if (G_UNLIKELY(!string)) {
		return FALSE;
	}
	return qahira_loader_supports_intern_string(self, string);
}

// private
gboolean
qahira_loader_load_start(QahiraLoader *self, GError **error)
{
	return QAHIRA_LOADER_GET_CLASS(self)->load_start(self, error);
}

// private
gssize
qahira_loader_load_increment(QahiraLoader *self, guchar *buffer, gsize size,
		GError **error)
{
	return QAHIRA_LOADER_GET_CLASS(self)->
		load_increment(self, buffer, size, error);
}

// private
cairo_surface_t *
qahira_loader_load_finish(QahiraLoader *self, GError **error)
{
	return QAHIRA_LOADER_GET_CLASS(self)->load_finish(self, error);
}

// protected
gboolean
qahira_loader_supports_intern_string(QahiraLoader *self, const gchar *type)
{
	struct Private *priv = GET_PRIVATE(self);
	for (GSList *node = priv->types; node; node = node->next) {
		if (type == node->data) {
			return TRUE;
		}
	}
	return FALSE;
}

// protected
void
qahira_loader_add_type(QahiraLoader *self, const gchar *type)
{
	struct Private *priv = GET_PRIVATE(self);
	const gchar *string = g_intern_string(type);
	if (G_UNLIKELY(!string)) {
		return;
	}
	priv->types = g_slist_prepend(priv->types, (gpointer)string);
}

// protected
void
qahira_loader_add_static_type(QahiraLoader *self, const gchar *type)
{
	struct Private *priv = GET_PRIVATE(self);
	const gchar *string = g_intern_static_string(type);
	if (G_UNLIKELY(!string)) {
		return;
	}
	priv->types = g_slist_prepend(priv->types, (gpointer)string);
}

// protected
cairo_surface_t *
qahira_loader_surface_create(QahiraLoader *self, cairo_format_t format,
		gint width, gint height)
{
	struct Private *priv = GET_PRIVATE(self);
	if (G_UNLIKELY(!priv->factory)) {
		return NULL;
	}
	return qahira_surface_factory_create(priv->factory, format,
			width, height);
}

// protected
guchar *
qahira_loader_surface_get_data(QahiraLoader *self, cairo_surface_t *surface)
{
	struct Private *priv = GET_PRIVATE(self);
	if (G_UNLIKELY(!priv->factory)) {
		return NULL;
	}
	return qahira_surface_factory_get_data(priv->factory, surface);
}

// protected
gint
qahira_loader_surface_get_stride(QahiraLoader *self,
		cairo_surface_t *surface)
{
	struct Private *priv = GET_PRIVATE(self);
	if (G_UNLIKELY(!priv->factory)) {
		return 0;
	}
	return qahira_surface_factory_get_stride(priv->factory, surface);
}
