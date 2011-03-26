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

#ifndef QAHIRA_H
#define QAHIRA_H

#include <cairo.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <qahira/error.h>
#include <qahira/image.h>
#include <qahira/types.h>
#include <stdio.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_QAHIRA \
	(qahira_get_type())

#define QAHIRA(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_QAHIRA, Qahira))

#define QAHIRA_IS_QAHIRA(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_QAHIRA))

#define QAHIRA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_QAHIRA, QahiraClass))

#define QAHIRA_IS_QAHIRA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_QAHIRA))

#define QAHIRA_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_QAHIRA, \
		QahiraClass))

typedef struct QahiraClass_ QahiraClass;

struct Qahira_ {
	/*< private >*/
	GObject parent_instance;
	gpointer priv;
};

typedef QahiraImage *
(*QahiraGetImage)(Qahira *self, const gchar *mime);

struct QahiraClass_ {
	/*< private >*/
	GObjectClass parent_class;
	QahiraGetImage get_image;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
Qahira *
qahira_new(void);

G_GNUC_WARN_UNUSED_RESULT
cairo_surface_t *
qahira_load(Qahira *self, const gchar *filename, GError **error);

gboolean
qahira_save(Qahira *self, cairo_surface_t *surface, const gchar *filename,
		GError **error);

G_GNUC_WARN_UNUSED_RESULT
cairo_surface_t *
qahira_load_file(Qahira *self, GFile *file, GError **error);

gboolean
qahira_save_file(Qahira *self, cairo_surface_t *surface,
		GFile *file, GError **error);

QahiraImage *
qahira_get_image(Qahira *self, const gchar *mime);

G_END_DECLS

#endif // QAHIRA_H
