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

#ifndef QAHIRA_LOADER_H
#define QAHIRA_LOADER_H

#include <cairo.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <qahira/types.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_LOADER \
	(qahira_loader_get_type())

#define QAHIRA_LOADER(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_LOADER, \
		QahiraLoader))

#define QAHIRA_IS_LOADER(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_LOADER))

#define QAHIRA_LOADER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_LOADER, \
		QahiraLoaderClass))

#define QAHIRA_IS_LOADER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_LOADER))

#define QAHIRA_LOADER_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_LOADER, \
		QahiraLoaderClass))

typedef struct QahiraLoaderClass_ QahiraLoaderClass;

struct QahiraLoader_ {
	/*< private >*/
	GObject parent_instance;
	gpointer priv;
};

typedef gboolean
(*QahiraLoaderStart)(QahiraLoader *self, GError **error);

typedef gssize
(*QahiraLoaderLoadIncrement)(QahiraLoader *self, guchar *buffer, gsize size,
		GError **error);

typedef gboolean
(*QahiraLoaderSave)(QahiraLoader *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error);

typedef cairo_surface_t *
(*QahiraLoaderLoadFinish)(QahiraLoader *self);

typedef void
(*QahiraLoaderSaveFinish)(QahiraLoader *self);

struct QahiraLoaderClass_ {
	/*< private >*/
	GObjectClass parent_class;
	QahiraLoaderStart load_start;
	QahiraLoaderLoadIncrement load_increment;
	QahiraLoaderLoadFinish load_finish;
	QahiraLoaderStart save_start;
	QahiraLoaderSave save;
	QahiraLoaderSaveFinish save_finish;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_loader_get_type(void) G_GNUC_CONST;

void
qahira_loader_set_surface_factory(QahiraLoader *self,
		QahiraSurfaceFactory *factory);

QahiraSurfaceFactory *
qahira_loader_get_surface_factory(QahiraLoader *self);

cairo_surface_t *
qahira_loader_load(QahiraLoader *self,  GInputStream *stream,
		GCancellable *cancel, GError **error);

gboolean
qahira_loader_save(QahiraLoader *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error);

gsize
qahira_loader_get_color_profile(QahiraLoader *self, gpointer *profile);

gboolean
qahira_loader_supports(QahiraLoader *self, const gchar *type);

G_END_DECLS

#endif // QAHIRA_LOADER_H
