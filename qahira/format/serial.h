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

#ifndef QAHIRA_FORMAT_SERIAL_H
#define QAHIRA_FORMAT_SERIAL_H

#include <qahira/format.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_FORMAT_SERIAL \
	(qahira_format_serial_get_type())

#define QAHIRA_FORMAT_SERIAL(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_FORMAT_SERIAL, \
		QahiraFormatSerial))

#define QAHIRA_IS_FORMAT_SERIAL(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_FORMAT_SERIAL))

#define QAHIRA_FORMAT_SERIAL_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_FORMAT_SERIAL, \
		QahiraFormatSerialClass))

#define QAHIRA_IS_FORMAT_SERIAL_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_FORMAT_SERIAL))

#define QAHIRA_FORMAT_SERIAL_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_FORMAT_SERIAL, \
		QahiraFormatSerialClass))

typedef struct QahiraFormatSerial_ QahiraFormatSerial;

typedef struct QahiraFormatSerialClass_ QahiraFormatSerialClass;

struct QahiraFormatSerial_ {
	/*< private >*/
	QahiraFormat parent_instance;
	gpointer priv;
};

struct QahiraFormatSerialClass_ {
	/*< private >*/
	QahiraFormatClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_format_serial_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraFormat *
qahira_format_serial_new(void);

gsize
qahira_format_serial_get_size(QahiraFormat *self, cairo_surface_t *surface);

G_END_DECLS

#endif // QAHIRA_FORMAT_SERIAL_H
