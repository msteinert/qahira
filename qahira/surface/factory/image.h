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

#ifndef QAHIRA_IMAGE_SURFACE_FACTORY_H
#define QAHIRA_IMAGE_SURFACE_FACTORY_H

#include <qahira/surface/factory.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_IMAGE_SURFACE_FACTORY \
	(qahira_image_surface_factory_get_type())

#define QAHIRA_IMAGE_SURFACE_FACTORY(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), \
		QAHIRA_TYPE_IMAGE_SURFACE_FACTORY, QahiraImageSurfaceFactory))

#define QAHIRA_IS_IMAGE_SURFACE_FACTORY(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), \
		QAHIRA_TYPE_IMAGE_SURFACE_FACTORY))

#define QAHIRA_IMAGE_SURFACE_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_IMAGE_SURFACE_FACTORY, \
		QahiraImageSurfaceFactoryClass))

#define QAHIRA_IS_IMAGE_SURFACE_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_IMAGE_SURFACE_FACTORY))

#define QAHIRA_IMAGE_SURFACE_FACTORY_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), \
		QAHIRA_TYPE_IMAGE_SURFACE_FACTORY, QahiraSurfaceFactoryClass))

typedef struct QahiraImageSurfaceFactory_ QahiraImageSurfaceFactory;

typedef struct QahiraImageSurfaceFactoryClass_ QahiraImageSurfaceFactoryClass;

struct QahiraImageSurfaceFactory_ {
	/*< private >*/
	QahiraSurfaceFactory parent_instance;
};

struct QahiraImageSurfaceFactoryClass_ {
	/*< private >*/
	QahiraSurfaceFactoryClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_image_surface_factory_get_type(void) G_GNUC_CONST;

QahiraSurfaceFactory *
qahira_image_surface_factory_new(void);

G_END_DECLS

#endif // QAHIRA_IMAGE_SURFACE_FACTORY_H
