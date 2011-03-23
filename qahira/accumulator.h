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

/**
 * \brief Common accumulators for signal handlers
 * \author Michael Steinert <mike.steinert@gmail.com>
 */

#ifndef QAHIRA_ACCUMULATOR_H
#define QAHIRA_ACCUMULATOR_H

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * \brief Accumulate a pointer return value, stop if a non-NULL value is
 *        returned.
 */
G_GNUC_INTERNAL
gboolean
qahira_pointer_accumulator(GSignalInvocationHint *hint, GValue *accu,
		const GValue *value, gpointer data);

/**
 * \brief Accumulate an integer return value, stop if a value greater than
 *        zero is returned.
 */
G_GNUC_INTERNAL
gboolean
qahira_integer_accumulator(GSignalInvocationHint *hint, GValue *accu,
		const GValue *value, gpointer data);

/**
 * \brief Accumulate an object return value, stop if a non-NULL value is
 *        returned.
 */
G_GNUC_INTERNAL
gboolean
qahira_object_accumulator(GSignalInvocationHint *hint, GValue *accu,
		const GValue *value, gpointer data);

G_END_DECLS

#endif // QAHIRA_ACCUMULATOR_H
