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
const char* var_format =
"<span size=\"x-large\" font_family=\"DejaVu Sans Mono\">%i"
    "<span foreground=\"%s\">"
        "<i><b>%s</b></i>"
        "<sub>%i</sub>"
    "</span>"
"</span>";

/* Variables names */
const char* var_names[] = {"a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s",
    "t", "u", "v", "w", "x", "y", "z"};

/* Mapping colors
 * http://www.christianfaur.com/color/index.html
 */
const char* var_colors[] = {
    "#0000B4", /* A */
    "#AF0D66", /* B */
    "#92F846", /* C */
    "#FFC82F", /* D */
    "#FF7600", /* E */
    "#B9B9B9", /* F */
    "#EBEBDE", /* G */
    "#646464", /* H */
    "#FFFF00", /* I */
    "#371370", /* J */
    "#FFFF96", /* K */
    "#CA3E5E", /* L */
    "#CD913F", /* M */
    "#0C4B64", /* N */
    "#FF0000", /* O */
    "#AF9B32", /* P */
    "#000000", /* Q */
    "#254619", /* R */
    "#792187", /* S */
    "#538CD0", /* T */
    "#009A25", /* U */
    "#B2DCCD", /* V */
    "#FF98D5", /* W */
    "#00004A", /* X */
    "#AFC84A", /* Y */
    "#3F190C"  /* Z */
};


char* var_name(int c, int s);

#endif
