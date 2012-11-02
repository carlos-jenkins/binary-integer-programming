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

#include "bip.h"

bip_context* bip_context_new(int num_vars, int num_rest)
{
    /* Check input is correct */
    if(num_vars < 1) {
        return NULL;
    }

    /* Allocate structure */
    bip_context* c = (bip_context*) malloc(sizeof(bip_context));
    if(c == NULL) {
        return NULL;
    }

    /* Try to allocate dynamic memory */
    c->restrictions = NULL;
    if(num_rest > 0) {
        c->restrictions = matrix_new(num_rest, num_vars + 2, 0);
        if(c->restrictions == NULL) {
            free(c);
            return NULL;
        }
    }
    c->function = (int*) malloc(num_vars * sizeof(int));
    if(c->function == NULL) {
        matrix_free(c->restrictions);
        free(c);
        return NULL;
    }

    /* Initialize values */
    for(int i = 0; i < num_vars; i++) {
        c->function[i] = 0;
    }

    /* Common */
    c->status = -1;
    c->execution_time = 0.0;
    c->memory_required = matrix_sizeof(c->restrictions) +
                         (num_vars * sizeof(int)) +
                         sizeof(bip_context);
    c->report_buffer = tmpfile();
    if(c->report_buffer == NULL) {
        matrix_free(c->restrictions);
        free(c->function);
        free(c);
        return NULL;
    }

    return c;
}

void bip_context_free(bip_context* c)
{
    if(c->restrictions != NULL) {
        matrix_free(c->restrictions);
    }
    fclose(c->report_buffer);
    free(c->function);
    free(c);
    return;
}

bool implicit_enumeration(bip_context* c)
{
    /* Start counting time */
    GTimer* timer = g_timer_new();

    /* Variables */
    int v = c->num_vars;
    int alpha = INT_MAX;
    if(c->maximize) {
        alpha = INT_MIN;
    }

    /* Try to allocate memory */
    int* fixed = (int*) malloc(v * sizeof(int));
    if(fixed == NULL) {
        return false;
    }
    int* workplace = (int*) malloc(v * sizeof(int));
    if(workplace == NULL) {
        free(fixed);
        return false;
    }
    int* candidate = (int*) malloc(v * sizeof(int));
    if(candidate == NULL) {
        free(fixed);
        free(workplace);
        return false;
    }

    /* Initialize vectors */
    for(int i = 0; i < v; i++) {
        fixed[i]     = -1;
        workplace[i] = -1;
        candidate[i] = -1;
    }

    /* Solve problem */
    impl_aux(c, fixed, &alpha, workplace, candidate, 0);

    /* Check if problem was solved */
    /* FIXMEFIXME */

    /* Stop counting time */
    g_timer_stop(timer);
    c->execution_time = g_timer_elapsed(timer, NULL);
    g_timer_destroy(timer);
    return true;
}

void impl_aux(bip_context* c, int* fixed, int* alpha,
                              int* workplace, int* candidate, int level)
{
    //int bf = best_fit(c, fixed);
    //if(bf > alpha) {
        //alpha = bf;
        // Close node
    //}
    return;
}

//int best_fit(bip_context* c, int* fixed);
//bool check_fact(bip_context* c, int* fixed);
//bool check_future_fact(bip_context* c, int* fixed);
//bool check_restrictions(bip_context* c, int* vars);
