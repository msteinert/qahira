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

#ifndef QAHIRA_IMAGE_H
#define QAHIRA_IMAGE_H

#include <cairo.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <qahira/types.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_IMAGE \
	(qahira_image_get_type())

#define QAHIRA_IMAGE(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_IMAGE, \
		QahiraImage))

#define QAHIRA_IS_IMAGE(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_IMAGE))

#define QAHIRA_IMAGE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_IMAGE, \
		QahiraImageClass))

#define QAHIRA_IS_IMAGE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_IMAGE))

#define QAHIRA_IMAGE_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_IMAGE, \
		QahiraImageClass))

typedef struct QahiraImageClass_ QahiraImageClass;

struct QahiraImage_ {
	/*< private >*/
	GObject parent_instance;
	gpointer priv;
};

typedef cairo_surface_t *
(*QahiraImageLoad)(QahiraImage *self, GInputStream *stream,
		GCancellable *cancel, GError **error);

typedef gboolean
(*QahiraImageSave)(QahiraImage *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error);

typedef cairo_surface_t *
(*QahiraImageSurfaceCreate)(QahiraImage *self, cairo_format_t format,
		gint width, gint height);

typedef guchar *
(*QahiraImageSurfaceGetData)(QahiraImage *self, cairo_surface_t *surface);

typedef gint
(*QahiraImageSurfaceGetStride)(QahiraImage *self, cairo_surface_t *surface);

struct QahiraImageClass_ {
	/*< private >*/
	GObjectClass parent_class;
	QahiraImageLoad load;
	QahiraImageSave save;
	QahiraImageSurfaceCreate surface_create;
	QahiraImageSurfaceGetData surface_get_data;
	QahiraImageSurfaceGetStride surface_get_stride;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_image_get_type(void) G_GNUC_CONST;

cairo_surface_t *
qahira_image_load(QahiraImage *self, GInputStream *stream,
		GCancellable *cancel, GError **error);

gboolean
qahira_image_save(QahiraImage *self, cairo_surface_t *surface,
		GOutputStream *stream, GCancellable *cancel, GError **error);

gboolean
qahira_image_supports(QahiraImage *self, const gchar *type);

G_END_DECLS

#endif // QAHIRA_IMAGE_H
