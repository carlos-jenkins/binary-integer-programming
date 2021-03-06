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

#include "format.h"

const char* NUM_FORMAT =
"<span size=\"x-large\" font_family=\"DejaVu Sans Mono\">%i</span>";

const char* VAR_FORMAT =
"<span size=\"x-large\" font_family=\"DejaVu Sans Mono\">%i"
    "<span foreground=\"%s\">"
        "<i><b>%s</b></i>"
        "<sub>%i</sub>"
    "</span>"
"</span>";

const char* VAR_NAMES[] = {
    "a", "b", "c", "d", "e", "f", "g", "h", "i",
    "j", "k", "l", "m", "n", "o", "p", "q", "r",
    "s", "t", "u", "v", "w", "x", "y", "z"
};

const char* VAR_COLORS[] = {
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

char* var_name(int c, int i, bool s) /* Coeff, Index, Sign */
{
    if(i < 0) { i *= -1; }
    if(!s && (c < 0)) { c *= -1; }

    char* r = g_strdup_printf(VAR_FORMAT, c,
                              VAR_COLORS[i % VARS],
                              VAR_NAMES[i % VARS],
                              (i / VARS) + 1);
    return r;
}

char* num_name(int c, bool s) /* Coeff, Sign */
{
    if(!s && (c < 0)) { c *= -1; }

    char* r = g_strdup_printf(NUM_FORMAT, c);
    return r;
}
