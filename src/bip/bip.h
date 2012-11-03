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

#ifndef H_BIP
#define H_BIP

#include "utils.h"
#include "matrix.h"

#define LE -1
#define GE  1
#define EQ  0

/**
 * Binary integer programming context data structure.
 */
typedef struct {

    /* Common */
    int status;
    double execution_time;
    unsigned int memory_required;
    FILE* report_buffer;

    /* Data */
    int num_vars;
    bool maximize;
    int* function;
    int num_rest;
    matrix* restrictions;

} bip_context;

bip_context* bip_context_new(int num_vars, int num_rest);
void bip_context_free(bip_context* c);

/**
 * Perform Implicit Enumeration algorithm with given context.
 *
 * @param bip_context, the binary integer programming context data structure.
 * @return TRUE if execution was successful or FALSE if and error ocurred. Check
 *         'status' flag in context to know what went wrong.
 */
bool implicit_enumeration(bip_context* c);

void impl_aux(bip_context* c, int* fixed, int* alpha, int* workplace,
                              int* candidate, int level);
int reset_workplace(bip_context* c, int* fixed, int* workplace);
int best_fit(bip_context* c, int* fixed, int* workplace);
bool check_future_fact(bip_context* c, int* fixed, int* workplace);
bool check_restrictions(bip_context* c, int* vars);

#endif
