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
#include "qahira/accumulator.h"

gboolean
qahira_pointer_accumulator(GSignalInvocationHint *hint, GValue *accu,
		const GValue *value, gpointer data)
{
	gpointer pointer = g_value_get_pointer(value);
	g_value_set_pointer(accu, pointer);
	return pointer ? FALSE : TRUE;
}

gboolean
qahira_integer_accumulator(GSignalInvocationHint *hint, GValue *accu,
		const GValue *value, gpointer data)
{
	gint integer = g_value_get_int(value);
	g_value_set_int(accu, integer);
	return integer > 0 ? FALSE : TRUE;
}

gboolean
qahira_object_accumulator(GSignalInvocationHint *hint, GValue *accu,
		const GValue *value, gpointer data)
{
	GObject *object = g_value_get_object(value);
	g_value_set_object(accu, object);
	return object ? FALSE : TRUE;
}
