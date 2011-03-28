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

#ifndef QAHIRA_FORMAT_PNG_H
#define QAHIRA_FORMAT_PNG_H

#include <qahira/format.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_FORMAT_PNG \
	(qahira_format_png_get_type())

#define QAHIRA_FORMAT_PNG(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_FORMAT_PNG, \
		QahiraFormatPng))

#define QAHIRA_IS_FORMAT_PNG(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_FORMAT_PNG))

#define QAHIRA_FORMAT_PNG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_FORMAT_PNG, \
		QahiraFormatPngClass))

#define QAHIRA_IS_FORMAT_PNG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_FORMAT_PNG))

#define QAHIRA_FORMAT_PNG_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_FORMAT_PNG, \
		QahiraFormatPngClass))

typedef struct QahiraFormatPng_ QahiraFormatPng;

typedef struct QahiraFormatPngClass_ QahiraFormatPngClass;

struct QahiraFormatPng_ {
	/*< private >*/
	QahiraFormat parent_instance;
	gpointer priv;
};

struct QahiraFormatPngClass_ {
	/*< private >*/
	QahiraFormatClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_format_png_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraFormat *
qahira_format_png_new(void);

void
qahira_format_png_set_compression(QahiraFormat *self, gint compression);

gint
qahira_format_png_get_compression(QahiraFormat *self);

void
qahira_format_set_interlace(QahiraFormat *self, gboolean interlace);

gboolean
qahira_format_get_interlace(QahiraFormat *self);

G_END_DECLS

#endif // QAHIRA_FORMAT_PNG_H
