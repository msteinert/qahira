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
#include <gio/gio.h>
#include "qahira/loader/factory.h"
#if QAHIRA_HAS_JPEG
#include "qahira/loader/jpeg.h"
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
#include "qahira/loader/png.h"
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
#include "qahira/loader/targa.h"
#endif // QAHIRA_HAS_TARGA

G_DEFINE_TYPE(QahiraLoaderFactory, qahira_loader_factory, G_TYPE_OBJECT)

static void
qahira_loader_factory_init(QahiraLoaderFactory *self)
{
}

static void
qahira_loader_factory_class_init(QahiraLoaderFactoryClass *klass)
{
}

QahiraLoaderFactory *
qahira_loader_factory_new(void)
{
	return g_object_new(QAHIRA_TYPE_LOADER_FACTORY, NULL);
}

QahiraLoader *
qahira_loader_factory_create(QahiraLoaderFactory *self, const gchar *mime)
{
#if QAHIRA_HAS_JPEG
	if (g_content_type_equals(mime, "image/jpeg")) {
		return qahira_loader_jpeg_new();
	}
#endif // QAHIRA_HAS_JPEG
#if QAHIRA_HAS_PNG
	if (g_content_type_equals(mime, "image/png")) {
		return qahira_loader_png_new();
	}
#endif // QAHIRA_HAS_PNG
#if QAHIRA_HAS_TARGA
	if (g_content_type_equals(mime, "image/x-tga")) {
		return qahira_loader_targa_new();
	}
#endif // QAHIRA_HAS_TARGA
	return NULL;
}
