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

#ifndef H_UTILS
#define H_UTILS

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <glib.h>

#define PLUS_INF FLT_MAX
#define MINUS_INF -FLT_MAX

#define F_EPSILON 0.000001

bool file_exists(char *fname);
char* read_file(char* fname);

char* get_current_time();

bool fequal(float a, float b);
int max(int a, int b);
int min(int a, int b);

bool copy_streams(FILE* input, FILE* output);
bool insert_file(char* filename, FILE* output);

bool is_empty_string(char* string);

char* sequence_name(int s);

char* get_line();

#endif
