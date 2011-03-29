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

/**
 * \brief Private/protected definitions for QahiraFormat
 * \author Michael Steinert <mike.steinert@gmail.com>
 */

#ifndef QAHIRA_FORMAT_PRIVATE_H
#define QAHIRA_FORMAT_PRIVATE_H

#include <qahira/format.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
void
qahira_format_add_type(QahiraFormat *self, const gchar *type);

G_GNUC_INTERNAL
void
qahira_format_add_static_type(QahiraFormat *self, const gchar *type);

G_GNUC_INTERNAL
gboolean
qahira_format_supports_intern_string(QahiraFormat *self, const gchar *type);

G_GNUC_INTERNAL
cairo_surface_t *
qahira_format_surface_create(QahiraFormat *self, cairo_format_t format,
		gint width, gint height);

G_GNUC_INTERNAL
guchar *
qahira_format_surface_get_data(QahiraFormat *self, cairo_surface_t *surface);

G_GNUC_INTERNAL
gint
qahira_format_surface_get_stride(QahiraFormat *self,
		cairo_surface_t *surface);

G_GNUC_INTERNAL
void
qahira_surface_size(cairo_surface_t *surface, gint *width, gint *height);

G_END_DECLS

#endif // QAHIRA_FORMAT_PRIVATE_H
