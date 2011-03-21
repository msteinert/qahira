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

#ifndef QAHIRA_LOADER_PRIVATE_H
#define QAHIRA_LOADER_PRIVATE_H

#include <qahira/loader.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
gboolean
qahira_loader_load_start(QahiraLoader *self, GError **error);

G_GNUC_INTERNAL
gssize
qahira_loader_load_increment(QahiraLoader *self, guchar *buffer, gsize size,
		GError **error);

G_GNUC_INTERNAL
cairo_surface_t *
qahira_loader_load_finish(QahiraLoader *self, GError **error);

G_GNUC_INTERNAL
gboolean
qahira_loader_supports_intern_string(QahiraLoader *self, const gchar *type);

G_GNUC_INTERNAL
void
qahira_loader_add_type(QahiraLoader *self, const gchar *type);

G_GNUC_INTERNAL
void
qahira_loader_add_static_type(QahiraLoader *self, const gchar *type);

G_GNUC_INTERNAL
cairo_surface_t *
qahira_loader_surface_create(QahiraLoader *self, cairo_format_t format,
		gint width, gint height);

G_GNUC_INTERNAL
guchar *
qahira_loader_surface_get_data(QahiraLoader *self, cairo_surface_t *surface);

G_GNUC_INTERNAL
gint
qahira_loader_surface_get_stride(QahiraLoader *self,
		cairo_surface_t *surface);

G_END_DECLS

#endif /* QAHIRA_LOADER_PRIVATE_H */
