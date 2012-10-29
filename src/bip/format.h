/*
 * Copyright (C) 2012 Carolina Aguilar <caroagse@gmail.com>
 * Copyright (C) 2012 Carlos Jenkins <carlos@jenkins.co.cr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_FORMAT
#define H_FORMAT

#include <stdbool.h>
#include <glib.h>

/* Lesser or equal and greater or equal unicode symbols */
#define LE "\u2264"
#define GE "\u2265"

#define VARS 26

/* Formatting string. Requires :
 *     - Coefficient (int, ex. 1),
 *     - Color (char*, ex "#8B0000"),
 *     - variable (char*, ex "a"),
 *     - Subindex (int, ex. 2).
 */
extern const char* var_format;

/* Variables names */
extern const char* var_names[];

/* Mapping colors
 * http://www.christianfaur.com/color/index.html
 */
extern const char* var_colors[];

/**
 * Format a variable.
 *
 * @param c, the coefficient of the variable.
 * @param i, variable number.
 * @param sign, if output should show the sign of the coefficient.
 * @return a string with the Pango markup for that variable.
 *         The string must be free'ed.
 */
char* var_name(int c, int i, bool sign);

#endif
