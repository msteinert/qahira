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

#ifndef QAHIRA_FORMAT_JPEG_H
#define QAHIRA_FORMAT_JPEG_H

#include <qahira/format.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_FORMAT_JPEG \
	(qahira_format_jpeg_get_type())

#define QAHIRA_FORMAT_JPEG(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_FORMAT_JPEG, \
		QahiraFormatJpeg))

#define QAHIRA_IS_FORMAT_JPEG(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_FORMAT_JPEG))

#define QAHIRA_FORMAT_JPEG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_FORMAT_JPEG, \
		QahiraFormatJpegClass))

#define QAHIRA_IS_FORMAT_JPEG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_FORMAT_JPEG))

#define QAHIRA_FORMAT_JPEG_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_FORMAT_JPEG, \
		QahiraFormatJpegClass))

typedef struct QahiraFormatJpeg_ QahiraFormatJpeg;

typedef struct QahiraFormatJpegClass_ QahiraFormatJpegClass;

struct QahiraFormatJpeg_ {
	/*< private >*/
	QahiraFormat parent_instance;
	gpointer priv;
};

typedef void
(*QahiraFormatJpegProgressive)(QahiraFormat *self, cairo_surface_t *surface);

struct QahiraFormatJpegClass_ {
	/*< private >*/
	QahiraFormatClass parent_class;
	/*< public >*/
	QahiraFormatJpegProgressive progressive;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_format_jpeg_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraFormat *
qahira_format_jpeg_new(void);

void
qahira_format_jpeg_set_quality(QahiraFormat *self, gint quality);

gint
qahira_format_jpeg_get_quality(QahiraFormat *self);

G_END_DECLS

#endif // QAHIRA_FORMAT_JPEG_H
