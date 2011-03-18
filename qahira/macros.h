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

#ifndef QAHIRA_MACROS_H
#define QAHIRA_MACROS_H

#include <glib.h>
#include "qahira/error.h"

G_BEGIN_DECLS

#ifdef G_DISABLE_CHECKS
#define qahira_return_error_if_fail(expr, val, error)
#else // G_DISABLE_CHECKS
#ifdef __GNUC__
#define qahira_return_error_if_fail(expr, val, error) \
G_STMT_START { \
        if G_UNLIKELY(!(expr)) { \
                g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE, \
                                "%s: assertion `%s' failed", \
                                G_STRLOC, #expr); \
                g_return_if_fail_warning(G_LOG_DOMAIN, G_STRLOC, #expr); \
                return val; \
        } \
} G_STMT_END
#else // __GNUC__
#define qahira_return_error_if_fail(expr, val, error) \
G_STMT_START { \
        if G_UNLIKELY(!(expr)) { \
                g_set_error(error, QAHIRA_ERROR, QAHIRA_ERROR_FAILURE, \
                                "file %s: line %s: assertion `%s' failed", \
                                __FILE__, __LINE__, #expr); \
                g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, \
                                "file %s: line %d: assertion `%s' failed", \
                                __FILE__, __LINE__, #expr); \
                return val; \
        } \
} G_STMT_END
#endif // __GNUC__
#endif // G_DISABLE_CHECKS

G_END_DECLS

#endif // CAIRO_QAHIRA_MACROS_H
