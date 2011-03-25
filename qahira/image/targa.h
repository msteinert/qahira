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

#ifndef QAHIRA_IMAGE_TARGA_H
#define QAHIRA_IMAGE_TARGA_H

#include <qahira/image.h>

G_BEGIN_DECLS

#define QAHIRA_TYPE_IMAGE_TARGA \
	(qahira_image_targa_get_type())

#define QAHIRA_IMAGE_TARGA(instance) \
	(G_TYPE_CHECK_INSTANCE_CAST((instance), QAHIRA_TYPE_IMAGE_TARGA, \
		QahiraImageTarga))

#define QAHIRA_IS_IMAGE_TARGA(instance) \
	(G_TYPE_CHECK_INSTANCE_TYPE((instance), QAHIRA_TYPE_IMAGE_TARGA))

#define QAHIRA_IMAGE_TARGA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), QAHIRA_TYPE_IMAGE_TARGA, \
		QahiraImageTargaClass))

#define QAHIRA_IS_IMAGE_TARGA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), QAHIRA_TYPE_IMAGE_TARGA))

#define QAHIRA_IMAGE_TARGA_GET_CLASS(instance) \
	(G_TYPE_INSTANCE_GET_CLASS((instance), QAHIRA_TYPE_IMAGE_TARGA, \
		QahiraImageTargaClass))

typedef enum {
	QAHIRA_ORIENTATION_TOP,
	QAHIRA_ORIENTATION_BOTTOM,
	QAHIRA_ORIENTATION_LEFT,
	QAHIRA_ORIENTATION_RIGHT
} QahiraOrientation;

typedef struct QahiraImageTarga_ QahiraImageTarga;

typedef struct QahiraImageTargaClass_ QahiraImageTargaClass;

struct QahiraImageTarga_ {
	/*< private >*/
	QahiraImage parent_instance;
	gpointer priv;
};

struct QahiraImageTargaClass_ {
	/*< private >*/
	QahiraImageClass parent_class;
};

G_GNUC_NO_INSTRUMENT
GType
qahira_image_targa_get_type(void) G_GNUC_CONST;

G_GNUC_WARN_UNUSED_RESULT
QahiraImage *
qahira_image_targa_new(void);

G_END_DECLS

#endif // QAHIRA_IMAGE_TARGA_H
