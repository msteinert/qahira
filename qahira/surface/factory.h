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

#ifndef QAHIRA_SURFACE_FACTORY_H
#define QAHIRA_SURFACE_FACTORY_H

#include <cairo.h>
#include <glib-object.h>
#include <qahira/types.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_SURFACE_FACTORY \
	(qahira_surface_factory_get_type())

#define QAHIRA_SURFACE_FACTORY(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_SURFACE_FACTORY, \
		QahiraSurfaceFactory))

#define QAHIRA_IS_SURFACE_FACTORY(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_SURFACE_FACTORY))

#define QAHIRA_SURFACE_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_SURFACE_FACTORY, \
		QahiraSurfaceFactoryClass))

#define QAHIRA_IS_SURFACE_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_SURFACE_FACTORY))

#define QAHIRA_SURFACE_FACTORY_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_SURFACE_FACTORY, \
		QahiraSurfaceFactoryClass))

typedef struct QahiraSurfaceFactoryClass_ QahiraSurfaceFactoryClass;

struct QahiraSurfaceFactory_ {
	/*< private >*/
	GObject parent_instance;
	gpointer priv;
};

typedef cairo_surface_t *
(*QahiraSurfaceFactoryCreate)(QahiraSurfaceFactory *self,
		cairo_content_t content, gint width, gint height);

typedef guchar *
(*QahiraSurfaceFactoryGetData)(QahiraSurfaceFactory *self,
		cairo_surface_t *surface);

typedef cairo_surface_type_t
(*QahiraSurfaceFactoryType)(QahiraSurfaceFactory *self);

typedef gint
(*QahiraSurfaceFactoryGet)(QahiraSurfaceFactory *self,
		cairo_surface_t *surface);

typedef cairo_format_t
(*QahiraSurfaceFactoryGetFormat)(QahiraSurfaceFactory *self,
		cairo_surface_t *surface);

struct QahiraSurfaceFactoryClass_ {
	/*< private >*/
	GObjectClass parent_class;
	/*< public >*/
	QahiraSurfaceFactoryCreate create;
	QahiraSurfaceFactoryGetData get_data;
	QahiraSurfaceFactoryType type;
	QahiraSurfaceFactoryGet get_width;
	QahiraSurfaceFactoryGet get_height;
	QahiraSurfaceFactoryGet get_stride;
	QahiraSurfaceFactoryGetFormat get_format;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_surface_factory_get_type(void) G_GNUC_CONST;

/**
 * \brief Create a new surface.
 *
 * \param self [in] A surface factory object.
 */
cairo_surface_t *
qahira_surface_factory_create(QahiraSurfaceFactory *self,
		cairo_content_t content, gint width, gint height);

/**
 * \brief Get a pointer to surface pixel data.
 *
 * \param self [in] A surface factory object.
 * \param surface [in] A cairo surface. The type of this surface must be
 *                     the same as the value returned by
 *                     qahira_surface_factory_type() for \e self.
 */
guchar *
qahira_surface_factory_get_data(QahiraSurfaceFactory *self,
		cairo_surface_t *surface);

/**
 * \brief Get the type of surface a factory will create.
 *
 * \param self [in] A surface factory object.
 *
 * \return The type of surface returned by qahira_surface_factory_create().
 */
cairo_surface_type_t
qahira_surface_factory_type(QahiraSurfaceFactory *self);

G_END_DECLS

#endif // QAHIRA_SURFACE_FACTORY_H
