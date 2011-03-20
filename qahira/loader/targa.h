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

#ifndef QAHIRA_LOADER_TARGA_H
#define QAHIRA_LOADER_TARGA_H

#include <qahira/loader.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_LOADER_TARGA \
	(qahira_loader_targa_get_type())

#define QAHIRA_LOADER_TARGA(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_LOADER_TARGA, \
		QahiraLoaderTarga))

#define QAHIRA_IS_LOADER_TARGA(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_LOADER_TARGA))

#define QAHIRA_LOADER_TARGA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_LOADER_TARGA, \
		QahiraLoaderTargaClass))

#define QAHIRA_IS_LOADER_TARGA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_LOADER_TARGA))

#define QAHIRA_LOADER_TARGA_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_LOADER_TARGA, \
		QahiraLoaderTargaClass))

typedef struct QahiraLoaderTarga_ QahiraLoaderTarga;

typedef struct QahiraLoaderTargaClass_ QahiraLoaderTargaClass;

struct QahiraLoaderTarga_ {
	/*< private >*/
	QahiraLoader parent_instance;
	gpointer priv;
};

struct QahiraLoaderTargaClass_ {
	/*< private >*/
	QahiraLoaderClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_loader_targa_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraLoader *
qahira_loader_targa_new(void);

G_END_DECLS

#endif // QAHIRA_LOADER_TARGA_H
