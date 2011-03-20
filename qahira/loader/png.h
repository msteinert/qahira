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

#ifndef QAHIRA_LOADER_PNG_H
#define QAHIRA_LOADER_PNG_H

#include <qahira/loader.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_LOADER_PNG \
	(qahira_loader_png_get_type())

#define QAHIRA_LOADER_PNG(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_LOADER_PNG, \
		QahiraLoaderPng))

#define QAHIRA_IS_LOADER_PNG(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_LOADER_PNG))

#define QAHIRA_LOADER_PNG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_LOADER_PNG, \
		QahiraLoaderPngClass))

#define QAHIRA_IS_LOADER_PNG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_LOADER_PNG))

#define QAHIRA_LOADER_PNG_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_LOADER_PNG, \
		QahiraLoaderPngClass))

typedef struct QahiraLoaderPng_ QahiraLoaderPng;

typedef struct QahiraLoaderPngClass_ QahiraLoaderPngClass;

struct QahiraLoaderPng_ {
	/*< private >*/
	QahiraLoader parent_instance;
	gpointer priv;
};

struct QahiraLoaderPngClass_ {
	/*< private >*/
	QahiraLoaderClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_loader_png_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraLoader *
qahira_loader_png_new(void);

G_END_DECLS

#endif // QAHIRA_LOADER_PNG_H
