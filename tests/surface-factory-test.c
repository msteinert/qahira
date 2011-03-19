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
#include <glib.h>
#include "qahira/surface/factory/image.h"
#include "qahira/surface/factory.h"

#define CLASS "/qahira/surface/factory"

static void
test_image(gpointer *fixture, gconstpointer data)
{
	QahiraSurfaceFactory *image = qahira_image_surface_factory_new();
	g_assert(image);
	cairo_surface_type_t type = qahira_surface_factory_type(image);
	g_assert_cmpint(type, ==, CAIRO_SURFACE_TYPE_IMAGE);
	cairo_surface_t *surface = qahira_surface_factory_create(image,
			CAIRO_FORMAT_ARGB32, 100, 100);
	g_assert(surface);
	cairo_status_t status = cairo_surface_status(surface);
	g_assert_cmpint(status, ==, CAIRO_STATUS_SUCCESS);
	gint width = qahira_surface_factory_get_width(image, surface);
	g_assert_cmpint(width, ==, 100);
	gint height = qahira_surface_factory_get_height(image, surface);
	g_assert_cmpint(height, ==, 100);
	cairo_format_t format =
		qahira_surface_factory_get_format(image, surface);
	g_assert_cmpint(format, ==, CAIRO_FORMAT_ARGB32);
	gint computed =
		cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, 100);
	gint stride = qahira_surface_factory_get_stride(image, surface);
	g_assert_cmpint(computed, ==, stride);
	guchar *pixels = qahira_surface_factory_get_data(image, surface);
	g_assert(pixels);
	cairo_surface_destroy(surface);
	g_object_unref(image);
}

int
main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);
	g_type_init();
	g_test_add(CLASS "/image", gpointer, NULL, NULL, test_image, NULL);
	return g_test_run();
}
