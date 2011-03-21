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

#ifndef QAHIRA_ERROR_H
#define QAHIRA_ERROR_H

#include <glib.h>

G_BEGIN_DECLS

#define QAHIRA_ERROR qahira_error_quark()

typedef enum {
	QAHIRA_ERROR_FAILURE,
	QAHIRA_ERROR_UNSUPPORTED,
	QAHIRA_ERROR_EMPTY_FILE,
	QAHIRA_ERROR_NO_MEMORY,
	QAHIRA_ERROR_CORRUPT_IMAGE,
	QAHIRA_ERROR_MAX
} QahiraError;

GQuark
qahira_error_quark(void);

G_END_DECLS

#endif // QAHIRA_ERROR_H
