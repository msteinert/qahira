/* Copyright 1999 Michael Zucchi
 * Copyright 1999 The Free Software Foundation
 * Copyright 1999 Red Hat, Inc.
 * Copyright 2011 Michael Steinert
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
#include "qahira/error.h"
#include "qahira/loader/jpeg.h"
#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <jerror.h>

G_DEFINE_TYPE(QahiraLoaderJpeg, qahira_loader_jpeg, QAHIRA_TYPE_LOADER)

#define ASSIGN_PRIVATE(instance) \
	(G_TYPE_INSTANCE_GET_PRIVATE(instance, QAHIRA_TYPE_LOADER_JPEG, \
		struct Private))

#define GET_PRIVATE(instance) \
	((struct Private *)((QahiraLoaderJpeg *)instance)->priv)

struct Private {
	struct jpeg_error_mgr error_mgr;
	struct jpeg_decompress_struct decompress;
	sigjmp_buf env;
	GError **error;
	cairo_surface_t *surface;
	gchar buffer[JMSG_LENGTH_MAX];
};

G_GNUC_NORETURN
static void
error_exit(j_common_ptr ptr)
{
	struct Private *priv = (struct Private *)ptr;
	priv->error_mgr.format_message(ptr, priv->buffer);
	g_set_error(priv->error, QAHIRA_ERROR,
			ptr->err->msg_code == JERR_OUT_OF_MEMORY
				? QAHIRA_ERROR_NO_MEMORY
				: QAHIRA_ERROR_CORRUPT_IMAGE,
			Q_("jpeg: %s"), priv->buffer);
	if (priv->surface) {
		cairo_surface_destroy(priv->surface);
		priv->surface = NULL;
	}
	siglongjmp(priv->env, 1);
}

G_GNUC_CONST
static void
output_message(j_common_ptr ptr)
{
	// do nothing
}

static void
qahira_loader_jpeg_init(QahiraLoaderJpeg *self)
{
	self->priv = ASSIGN_PRIVATE(self);
	struct Private *priv = GET_PRIVATE(self);
	priv->error_mgr.error_exit = error_exit;
	priv->error_mgr.output_message = output_message;
}

static void
finalize(GObject *base)
{
	struct Private *priv = GET_PRIVATE(base);
	if (priv->surface) {
		cairo_surface_destroy(priv->surface);
	}
	G_OBJECT_CLASS(qahira_loader_jpeg_parent_class)->finalize(base);
}

static gboolean
load_start(QahiraLoader *self, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	priv->error = error;
	jpeg_std_error(&priv->error_mgr);
	if (sigsetjmp(priv->env, 1)) {
		return FALSE;
	}
	jpeg_create_decompress(&priv->decompress);
	return TRUE;
}

static gssize
load_increment(QahiraLoader *self, guchar *buffer, gsize size, GError **error)
{
	struct Private *priv = GET_PRIVATE(self);
	priv->error = error;
	if (sigsetjmp(priv->env, 1)) {
		return -1;
	}
	gint status;
	while (TRUE) {
		if (!priv->surface) {
			// read JPEG header
			status = jpeg_read_header(priv->decompress, TRUE);
		}
	}
	return size;
}

static cairo_surface_t *
load_finish(QahiraLoader *self)
{
	struct Private *priv = GET_PRIVATE(self);
	jpeg_destroy_decompress(&priv->decompress);
	GError *error = NULL;
	priv->error = &error;
	if (sigsetjmp(priv->env, 1)) {
		g_error_free(error);
		if (priv->surface) {
			cairo_surface_destroy(priv->surface);
			priv->surface = NULL;
		}
	} else {
		jpeg_finish_decompress(&priv->decompress);
	}
	jpeg_destroy_decompress(&priv->decompress);
	cairo_surface_t *surface = priv->surface;
	priv->surface = NULL;
	return surface;
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
qahira_loader_jpeg_class_init(QahiraLoaderJpegClass *klass)
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
qahira_loader_jpeg_new(void)
{
	return g_object_new(QAHIRA_TYPE_LOADER_JPEG, NULL);
}
