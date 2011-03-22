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
#include <cairo-xlib.h>
#include <glib.h>
#include "qahira/qahira.h"
#include <stdlib.h>
#include <X11/Xlib.h>

int
main(int argc, char *argv[])
{
	g_type_init();
	cairo_surface_t *surface = NULL;
	cairo_surface_t *image = NULL;
	int status = EXIT_SUCCESS;
	Display *display = NULL;
	GError *error = NULL;
	Window window = 0UL;
	cairo_t *cr = NULL;
	Qahira *qr = NULL;
	if (argc < 2) {
		g_message("image file not specified");
		goto error;
	}
	qr = qahira_new();
	if (!qr) {
		g_message("qahira_new returned NULL");
		goto error;
	}
	if (!qahira_set_filename(qr, argv[1], &error)) {
		goto error;
	}
	image = qahira_surface_create(qr, &error);
	if (!image) {
		goto error;
	}
	gint width = qahira_surface_get_width(qr, image);
	gint height = qahira_surface_get_height(qr, image);
	display = XOpenDisplay(NULL);
	if (!display) {
		g_message("failed to open display: %s", XDisplayName(NULL));
		goto error;
	}
	gint id = DefaultScreen(display);
	unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask;
	XSetWindowAttributes attr;
	attr.background_pixel = WhitePixel(display, id);
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
	window = XCreateWindow(display, RootWindow(display, id), 0, 0,
			width, height, 0, DefaultDepth(display, id),
			InputOutput, DefaultVisual(display, id), mask, &attr);
	if (!window) {
		g_message("failed to create window");
		goto error;
	}
	Atom delete = XInternAtom(display, "WM_DELETE_WINDOW", True);
	if (delete) {
		XSetWMProtocols(display, window, &delete, 1);
	}
	surface = cairo_xlib_surface_create(display, window,
			DefaultVisual(display, id), width, height);
	cr = cairo_create(surface);
	XMapWindow(display, window);
	XEvent event;
	while (TRUE) {
		XNextEvent(display, &event);
		switch (event.type) {
		case Expose:
			cairo_set_source_surface(cr, image, 0., 0.);
			cairo_paint(cr);
			break;
		case ClientMessage:
			goto exit;
		default:
			break;
		}
	}
exit:
	if (qr) {
		g_object_unref(qr);
	}
	if (cr) {
		cairo_destroy(cr);
	}
	if (image) {
		cairo_surface_destroy(image);
	}
	if (surface) {
		cairo_surface_destroy(surface);
	}
	if (window) {
		XDestroyWindow(display, window);
	}
	if (display) {
		XCloseDisplay(display);
	}
	return status;
error:
	if (error) {
		g_message("%s", error->message);
		g_error_free(error);
	}
	status = EXIT_FAILURE;
	goto exit;
}
