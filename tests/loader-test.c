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
#include "qahira/loader.h"

#define CLASS "/qahira/loader"

#if QAHIRA_HAS_JPEG
#include "qahira/loader/jpeg.h"
static void
test_jpeg(gpointer *fixture, gconstpointer data)
{
	QahiraLoader *jpeg = qahira_loader_jpeg_new();
	g_assert(jpeg);
	g_object_unref(jpeg);
}
#endif // QAHIRA_HAS_JPEG

#if QAHIRA_HAS_PNG
#include "qahira/loader/png.h"
static void
test_png(gpointer *fixture, gconstpointer data)
{
	QahiraLoader *png = qahira_loader_png_new();
	g_assert(png);
	g_object_unref(png);
}
#endif // QAHIRA_HAS_PNG

#if QAHIRA_HAS_TARGA
#include "qahira/loader/targa.h"
static void
test_targa(gpointer *fixture, gconstpointer data)
{
	QahiraLoader *targa = qahira_loader_targa_new();
	g_assert(targa);
	g_object_unref(targa);
}
#endif // QAHIRA_HAS_TARGA

int
main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);
	g_type_init();
#if QAHIRA_HAS_JPEG
	g_test_add(CLASS "/jpeg", gpointer, NULL, NULL, test_jpeg, NULL);
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
	g_test_add(CLASS "/png", gpointer, NULL, NULL, test_png, NULL);
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
	g_test_add(CLASS "/targa", gpointer, NULL, NULL, test_targa, NULL);
#endif // QAHIRA_HAS_TARGA
	return g_test_run();
}
