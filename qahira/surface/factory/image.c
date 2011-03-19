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
#include "qahira/surface/factory/image.h"

G_DEFINE_TYPE(QahiraImageSurfaceFactory, qahira_image_surface_factory,
		QAHIRA_TYPE_SURFACE_FACTORY)

static void
qahira_image_surface_factory_init(QahiraImageSurfaceFactory *self)
{
}

static cairo_surface_t *
create(QahiraSurfaceFactory *self, cairo_content_t content,
		gint width, gint height)
{
	return cairo_image_surface_create(content, width, height);
}

static guchar *
get_data(QahiraSurfaceFactory *self, cairo_surface_t *surface)
{
	return cairo_image_surface_get_data(surface);
}

static cairo_surface_type_t
type(QahiraSurfaceFactory *self)
{
	return CAIRO_SURFACE_TYPE_IMAGE;
}

static gint
get_width(QahiraSurfaceFactory *self, cairo_surface_t *surface)
{
	return cairo_image_surface_get_width(surface);
}

static gint
get_height(QahiraSurfaceFactory *self, cairo_surface_t *surface)
{
	return cairo_image_surface_get_height(surface);
}

static gint
get_stride(QahiraSurfaceFactory *self, cairo_surface_t *surface)
{
	return cairo_image_surface_get_stride(surface);
}

static cairo_format_t
get_format(QahiraSurfaceFactory *self, cairo_surface_t *surface)
{
	return cairo_image_surface_get_format(surface);
}

static void
qahira_image_surface_factory_class_init(QahiraImageSurfaceFactoryClass *klass)
{
	QahiraSurfaceFactoryClass *surface_factory_class =
		QAHIRA_SURFACE_FACTORY_CLASS(klass);
	surface_factory_class->create = create;
	surface_factory_class->get_data = get_data;
	surface_factory_class->type = type;
	surface_factory_class->get_width = get_width;
	surface_factory_class->get_height = get_height;
	surface_factory_class->get_stride = get_stride;
	surface_factory_class->get_format = get_format;
}

QahiraSurfaceFactory *
qahira_image_surface_factory_new(void)
{
	return g_object_new(QAHIRA_TYPE_IMAGE_SURFACE_FACTORY, NULL);
}
