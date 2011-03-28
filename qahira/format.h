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

/**
 * \file
 * \brief Base class for image formats
 * \author Michael Steinert <mike.steinert@gmail.com>
 */

#ifndef QAHIRA_FORMAT_H
#define QAHIRA_FORMAT_H

#include <cairo.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <qahira/types.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_FORMAT \
	(qahira_format_get_type())

#define QAHIRA_FORMAT(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_FORMAT, \
		QahiraFormat))

#define QAHIRA_IS_FORMAT(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_FORMAT))

#define QAHIRA_FORMAT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_FORMAT, \
		QahiraFormatClass))

#define QAHIRA_IS_FORMAT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_FORMAT))

#define QAHIRA_FORMAT_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_FORMAT, \
		QahiraFormatClass))

typedef struct QahiraFormatClass_ QahiraFormatClass;

struct QahiraFormat_ {
	/*< private >*/
	GObject parent_instance;
	gpointer priv;
};

typedef cairo_surface_t *
(*QahiraFormatLoad)(QahiraFormat *self, GInputStream *stream,
		GCancellable *cancel, GError **error);

typedef gboolean
(*QahiraFormatSave)(QahiraFormat *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error);

typedef cairo_surface_t *
(*QahiraFormatSurfaceCreate)(QahiraFormat *self, cairo_format_t format,
		gint width, gint height);

typedef guchar *
(*QahiraFormatSurfaceGetData)(QahiraFormat *self, cairo_surface_t *surface);

typedef gint
(*QahiraFormatSurfaceGetStride)(QahiraFormat *self, cairo_surface_t *surface);

struct QahiraFormatClass_ {
	/*< private >*/
	GObjectClass parent_class;
	QahiraFormatLoad load;
	QahiraFormatSave save;
	QahiraFormatSurfaceCreate surface_create;
	QahiraFormatSurfaceGetData surface_get_data;
	QahiraFormatSurfaceGetStride surface_get_stride;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_format_get_type(void) G_GNUC_CONST;

cairo_surface_t *
qahira_format_load(QahiraFormat *self, GInputStream *stream,
		GCancellable *cancel, GError **error);

gboolean
qahira_format_save(QahiraFormat *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error);

gboolean
qahira_format_supports(QahiraFormat *self, const gchar *type);

G_END_DECLS

#endif // QAHIRA_FORMAT_H
