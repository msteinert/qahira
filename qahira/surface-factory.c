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
#include "qahira/surface/factory.h"

G_DEFINE_ABSTRACT_TYPE(QahiraSurfaceFactory, qahira_surface_factory,
		G_TYPE_OBJECT)

static void
qahira_surface_factory_init(QahiraSurfaceFactory *self)
{
}

static void
qahira_surface_factory_class_init(QahiraSurfaceFactoryClass *klass)
{
}

cairo_surface_t *
qahira_surface_factory_create(QahiraSurfaceFactory *self,
		cairo_content_t content, gint width, gint height,
		gint stride)
{
	g_return_val_if_fail(QAHIRA_IS_SURFACE_FACTORY(self), NULL);
	return QAHIRA_SURFACE_FACTORY_GET_CLASS(self)->
		create(self, content, width, height, stride);
}

guint *
qahira_surface_factory_get_data(QahiraSurfaceFactory *self,
		cairo_surface_t *surface)
{
	g_return_val_if_fail(QAHIRA_IS_SURFACE_FACTORY(self), NULL);
	g_return_val_if_fail(surface, NULL);
	if (qahira_surface_factory_type(self)
			!= cairo_surface_get_type(surface)) {
		return NULL;
	}
	return QAHIRA_SURFACE_FACTORY_GET_CLASS(self)->
		get_data(self, surface);
}

cairo_surface_type_t
qahira_surface_factory_type(QahiraSurfaceFactory *self)
{
	g_return_val_if_fail(QAHIRA_IS_SURFACE_FACTORY(self), -1);
	return QAHIRA_SURFACE_FACTORY_GET_CLASS(self)->type(self);
}
