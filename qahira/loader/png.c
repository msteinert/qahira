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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "qahira/loader/png.h"

G_DEFINE_TYPE(QahiraLoaderPng, qahira_loader_png, QAHIRA_TYPE_LOADER)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_LOADER_PNG, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraLoaderPng *)instance)->priv)

struct Private {
	gint ignored;
};

static void
qahira_loader_png_init(QahiraLoaderPng *self)
{
	self->priv = ASSIGN_PRIVATE(self);
}

static void
finalize(GObject *base)
{
	//struct Private *priv = GET_PRIVATE(base);
	G_OBJECT_CLASS(qahira_loader_png_parent_class)->finalize(base);
}

static gboolean
load_start(QahiraLoader *self, GError **error)
{
	// TODO
	return FALSE;
}

static gssize
load_increment(QahiraLoader *self, guchar *buffer, gsize size, GError **error)
{
	// TODO
	return 0;
}

static cairo_surface_t *
load_finish(QahiraLoader *self)
{
	// TODO
	return NULL;
}

static gboolean
save_start(QahiraLoader *self, GError **error)
{
	// TODO
	return FALSE;
}

static gboolean        
save(QahiraLoader *self, cairo_surface_t *surface, GOutputStream *stream,
		GCancellable *cancel, GError **error)
{
	// TODO
	return FALSE;
}

static void
save_finish(QahiraLoader *self)
{
	// TODO
}

static void
qahira_loader_png_class_init(QahiraLoaderPngClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = finalize;
	QahiraLoaderClass *loader_class = QAHIRA_LOADER_CLASS(klass);
	loader_class->load_start = load_start;
	loader_class->load_increment = load_increment;
	loader_class->load_finish = load_finish;
	loader_class->save_start = save_start;
	loader_class->save = save;
	loader_class->save_finish = save_finish;
	g_type_class_add_private(klass, sizeof(struct Private));
}

QahiraLoader *
qahira_loader_png_new(void)
{
	return g_object_new(QAHIRA_TYPE_LOADER_PNG, NULL);
}
