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
#include "qahira/qahira.h"

#define CLASS "/qahira/image"

static void
setup(GString **path, gconstpointer data)
{
	const gchar *srcdir = g_getenv("srcdir");
	*path = g_string_new(srcdir ? srcdir : ".");
	g_assert(*path);
	g_string_append(*path, "/../contrib/");
}

static void
teardown(GString **path, gconstpointer data)
{
	g_string_free(*path, TRUE);
}

static inline GInputStream *
open_input(const gchar *filename)
{
	GFile *file = g_file_new_for_path(filename);
	g_assert(file);
	GError *error = NULL;
	GInputStream *stream = (GInputStream *)
		g_file_read(file, NULL, &error);
	if (!stream) {
		g_message("%s: %s", filename, error->message);
		g_error_free(error);
		g_assert(stream);
	}
	g_object_unref(file);
	return stream;
}

#if QAHIRA_HAS_JPEG
#include "qahira/image/jpeg.h"
static void
test_jpeg(GString **path, gconstpointer data)
{
	QahiraImage *jpeg = qahira_image_jpeg_new();
	g_assert(jpeg);
	g_string_append(*path, "sphinx.jpg");
	GInputStream *stream = open_input((*path)->str);
	GError *error = NULL;
	cairo_surface_t *surface =
		qahira_image_load(jpeg, stream, NULL, &error);
	g_assert(surface);
	cairo_status_t status = cairo_surface_status(surface);
	g_assert_cmpint(status, ==, CAIRO_STATUS_SUCCESS);
	cairo_surface_destroy(surface);
	g_object_unref(stream);
	g_object_unref(jpeg);
}
#endif // QAHIRA_HAS_JPEG

#if QAHIRA_HAS_PNG
#include "qahira/image/png.h"
static void
test_png(GString **path, gconstpointer data)
{
	QahiraImage *png = qahira_image_png_new();
	g_assert(png);
	g_string_append(*path, "sphinx.png");
	GInputStream *stream = open_input((*path)->str);
	GError *error = NULL;
	cairo_surface_t *surface =
		qahira_image_load(png, stream, NULL, &error);
	g_assert(surface);
	cairo_status_t status = cairo_surface_status(surface);
	g_assert_cmpint(status, ==, CAIRO_STATUS_SUCCESS);
	cairo_surface_destroy(surface);
	g_object_unref(stream);
	g_object_unref(png);
}
#endif // QAHIRA_HAS_PNG

#if QAHIRA_HAS_TARGA
#include "qahira/image/targa.h"
static void
test_targa(GString **path, gconstpointer data)
{
	QahiraImage *targa = qahira_image_targa_new();
	g_assert(targa);
	g_string_append(*path, "sphinx.tga");
	GInputStream *stream = open_input((*path)->str);
	GError *error = NULL;
	cairo_surface_t *surface =
		qahira_image_load(targa, stream, NULL, &error);
	g_assert(surface);
	cairo_status_t status = cairo_surface_status(surface);
	g_assert_cmpint(status, ==, CAIRO_STATUS_SUCCESS);
	cairo_surface_destroy(surface);
	g_object_unref(stream);
	g_object_unref(targa);
}
#endif // QAHIRA_HAS_TARGA

int
main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);
	g_type_init();
#if QAHIRA_HAS_JPEG
	g_test_add(CLASS "/jpeg", GString *, NULL,
			setup, test_jpeg, teardown);
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
	g_test_add(CLASS "/png", GString *, NULL,
			setup, test_png, teardown);
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
	g_test_add(CLASS "/targa", GString *, NULL,
			setup, test_targa, teardown);
#endif // QAHIRA_HAS_TARGA
	return g_test_run();
}
