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
#include "qahira/loader/factory.h"

#define CLASS "/qahira/loader/factory"

G_GNUC_UNUSED
static void
setup(QahiraLoaderFactory **factory, gconstpointer data)
{
	*factory = qahira_loader_factory_new();
	g_assert(*factory);
}

G_GNUC_UNUSED
static void
teardown(QahiraLoaderFactory **factory, gconstpointer data)
{
	g_object_unref(*factory);
}

#if QAHIRA_HAS_JPEG
static void
test_jpeg(QahiraLoaderFactory **factory, gconstpointer data)
{
	QahiraLoader *jpeg =
		qahira_loader_factory_create(*factory, "image/jpeg");
	g_assert(jpeg);
	g_object_unref(jpeg);
}
#endif // QAHIRA_HAS_JPEG

#if QAHIRA_HAS_PNG
static void
test_png(QahiraLoaderFactory **factory, gconstpointer data)
{
	QahiraLoader *png =
		qahira_loader_factory_create(*factory, "image/png");
	g_assert(png);
	g_object_unref(png);
}
#endif // QAHIRA_HAS_PNG

#if QAHIRA_HAS_TARGA
static void
test_targa(QahiraLoaderFactory **factory, gconstpointer data)
{
	QahiraLoader *targa =
		qahira_loader_factory_create(*factory, "image/x-tga");
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
	g_test_add(CLASS "/jpeg", QahiraLoaderFactory *, NULL,
			setup, test_jpeg, teardown);
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
	g_test_add(CLASS "/png", QahiraLoaderFactory *, NULL,
			setup, test_png, teardown);
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
	g_test_add(CLASS "/targa", QahiraLoaderFactory *, NULL,
			setup, test_targa, teardown);
#endif // QAHIRA_HAS_TARGA
	return g_test_run();
}
