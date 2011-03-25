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

#define CLASS "/qahira"

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

static void
test(GString **path, gconstpointer data)
{
	Qahira *qr = qahira_new();
	g_assert(qr);
	GString *file;
	GError *error = NULL;
	cairo_surface_t *surface = NULL;
#if QAHIRA_HAS_JPEG
	file = g_string_new((*path)->str);
	g_assert(file);
	g_string_append(file, "sphinx.jpg");
	surface = qahira_load_filename(qr, file->str, NULL, &error);
	g_string_free(file, TRUE);
	if (!surface) {
		g_message("%s: %s", file->str, error->message);
		g_error_free(error);
		g_assert(surface);
	}
	cairo_surface_destroy(surface);
#endif
#if QAHIRA_HAS_PNG
	file = g_string_new((*path)->str);
	g_assert(file);
	g_string_append(file, "sphinx.png");
	surface = qahira_load_filename(qr, file->str, NULL, &error);
	g_string_free(file, TRUE);
	if (!surface) {
		g_message("%s: %s", file->str, error->message);
		g_error_free(error);
		g_assert(surface);
	}
	cairo_surface_destroy(surface);
#endif
#if QAHIRA_HAS_TARGA
	file = g_string_new((*path)->str);
	g_assert(file);
	g_string_append(file, "sphinx.tga");
	surface = qahira_load_filename(qr, file->str, NULL, &error);
	g_string_free(file, TRUE);
	if (!surface) {
		g_message("%s: %s", file->str, error->message);
		g_error_free(error);
		g_assert(surface);
	}
	cairo_surface_destroy(surface);
#endif
	g_object_unref(qr);
}

int
main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);
	g_type_init();
	g_test_add(CLASS, GString *, NULL, setup, test, teardown);
	return g_test_run();
}
