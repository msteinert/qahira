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

#ifndef QAHIRA_LOADER_FACTORY_H
#define QAHIRA_LOADER_FACTORY_H

#include <glib-object.h>
#include <qahira/types.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_LOADER_FACTORY \
	(qahira_loader_factory_get_type())

#define QAHIRA_LOADER_FACTORY(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_LOADER_FACTORY, \
		QahiraLoaderFactory))

#define QAHIRA_IS_LOADER_FACTORY(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_LOADER_FACTORY))

#define QAHIRA_LOADER_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_LOADER_FACTORY, \
		QahiraLoaderFactoryClass))

#define QAHIRA_IS_LOADER_FACTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_LOADER_FACTORY))

#define QAHIRA_LOADER_FACTORY_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_LOADER_FACTORY, \
		QahiraLoaderFactoryClass))

typedef struct QahiraLoaderFactoryClass_ QahiraLoaderFactoryClass;

struct QahiraLoaderFactory_ {
	/*< private >*/
	GObject parent_instance;
	gpointer priv;
};

struct QahiraLoaderFactoryClass_ {
	/*< private >*/
	GObjectClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_loader_factory_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraLoaderFactory *
qahira_loader_factory_new(void);

G_GNUC_WARN_UNUSED_RESULT
QahiraLoader *
qahira_loader_factory_create(QahiraLoaderFactory *self, const gchar *type);

G_END_DECLS

#endif // QAHIRA_LOADER_FACTORY_H
