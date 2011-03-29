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

#ifndef QAHIRA_UTILITY_H
#define QAHIRA_UTILITY_H

#include <glib.h>

G_BEGIN_DECLS

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define QAHIRA_A (3)
#define QAHIRA_R (2)
#define QAHIRA_G (1)
#define QAHIRA_B (0)
#else
#define QAHIRA_A (0)
#define QAHIRA_R (1)
#define QAHIRA_G (2)
#define QAHIRA_B (3)
#endif

gint
qahira_premultiply(gint alpha, gint color);

gint
qahira_unpremultiply(gint alpha, gint color);

G_END_DECLS

#endif // QAHIRA_UTILITY_H
