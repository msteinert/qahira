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
 * \brief Private/protected definitions for QahiraImage
 * \author Michael Steinert <mike.steinert@gmail.com>
 */

#ifndef QAHIRA_IMAGE_PRIVATE_H
#define QAHIRA_IMAGE_PRIVATE_H

#include <qahira/image.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
void
qahira_image_add_type(QahiraImage *self, const gchar *type);

G_GNUC_INTERNAL
void
qahira_image_add_static_type(QahiraImage *self, const gchar *type);

G_GNUC_INTERNAL
gboolean
qahira_image_supports_intern_string(QahiraImage *self, const gchar *type);

G_GNUC_INTERNAL
cairo_surface_t *
qahira_image_surface_create(QahiraImage *self, cairo_format_t format,
		gint width, gint height);

G_GNUC_INTERNAL
guchar *
qahira_image_surface_get_data(QahiraImage *self, cairo_surface_t *surface);

G_GNUC_INTERNAL
gint
qahira_image_surface_get_stride(QahiraImage *self, cairo_surface_t *surface);

G_END_DECLS

#endif // QAHIRA_IMAGE_PRIVATE_H
