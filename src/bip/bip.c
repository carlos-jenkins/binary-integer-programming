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
#include "report.h"

bip_context* bip_context_new(int num_vars, int num_rest)
{
    /* Check input is correct */
    if((num_vars < 1) || (num_rest < 0)) {
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

    /* Save variables */
    c->num_vars = num_vars;
    c->num_rest = num_rest;
    c->maximize = true;

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
    matrix_free(c->restrictions);
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
    int* parents = (int*) malloc(v * sizeof(int));
    if(parents == NULL) {
        free(fixed);
        free(workplace);
        free(candidate);
        return false;
    }

    /* Initialize vectors */
    for(int i = 0; i < v; i++) {
        fixed[i]     = -1;
        workplace[i] = -1;
        candidate[i] = -1;
        parents[i]   = -1;
    }

    /* Solve problem */
    int node = 1;
    impl_aux(c, fixed, &alpha, workplace, candidate, parents, 0, &node);

    /* Check if problem was solved */
    DEBUG("Problem resolution ended with the coefficients:\n");
    for(int i = 0; i < c->num_vars; i++) {
        DEBUG("%i ", candidate[i]);
    }
    DEBUG("\n");
    /* FIXMEFIXME */

    /* Stop counting time */
    g_timer_stop(timer);
    c->execution_time = g_timer_elapsed(timer, NULL);
    g_timer_destroy(timer);
    return true;
}

void impl_aux(bip_context* c, int* fixed, int* alpha, int* workplace,
              int* candidate, int* parents, int level, int* node)
{
    if(level == c->num_vars) {
        return;
    }

    /* Register node num */
    int c_node = (*node);
    (*node) = c_node + 1;
    parents[level] = c_node;
    imp_node_open(c, fixed, parents, c_node);

    /* Calculate best fit and test if performance is improved */
    int bf = best_fit(c, fixed, workplace);
    imp_node_log_bf(c, fixed, workplace, bf, *alpha); /* LOG */
    if((c->maximize && (bf <= *alpha)) || (!c->maximize && (bf >= *alpha))) {
        DEBUG("Node %i: Close node. Doesn't improve performance.\n", c_node);
        imp_node_close(c, doesnt_improve); /* LOG */
        return;
    }

    /* Check factibility */
    imp_node_log_rc(c); /* LOG */
    bool fact = check_restrictions(c, workplace);
    if(fact) {

        /* Set the solution as new candidate */
        for(int i = 0; i < c->num_vars; i++) {
            candidate[i] = workplace[i];
        }

        /* Set alpha as the new performance */
        (*alpha) = bf;

        DEBUG("Node %i: Close node. New candidate solution: %i.\n", c_node, bf);
        imp_node_close(c, new_candidate); /* LOG */
        return;
    }

    /* Not factible, check possible future factibility */
    imp_node_log_ff(c);
    bool future_fact = check_future_fact(c, fixed, workplace);
    if(!future_fact) {
        DEBUG("Node %i: Close node. Not factible.\n", c_node);
        imp_node_close(c, not_factible); /* LOG */
        return;
    }

    DEBUG("Node %i: Expand node. Possible future factibility.\n", c_node);
    imp_node_close(c, expand); /* LOG */

    fixed[level] = 0;
    impl_aux(c, fixed, alpha, workplace, candidate, parents, level + 1, node);
    for(int i = level + 1; i < c->num_vars; i++) {
        fixed[i] = -1;
        parents[i] = -1;
    }

    fixed[level] = 1;
    impl_aux(c, fixed, alpha, workplace, candidate, parents, level + 1, node);
    for(int i = level + 1; i < c->num_vars; i++) {
        fixed[i] = -1;
        parents[i] = -1;
    }

    return;
}

int reset_workplace(bip_context* c, int* fixed, int* workplace)
{
    /* Copy fixed variables to the workplace */
    int i = 0;
    for(; i < c->num_vars; i++) {
        int n = fixed[i];
        if(n == -1) {
            break;
        }
        workplace[i] = n;
    }

    for(int j = i; j < c->num_vars; j++) {
        workplace[j] = -1;
    }

    return i;
}

int dot_product(int* vector1, int* vector2, int size)
{
    int dp = 0;
    for(int i = 0; i < size; i++) {
        dp = dp + (vector1[i] * vector2[i]);
    }
    return dp;
}

int best_fit(bip_context* c, int* fixed, int* workplace)
{
    /* Flush fixed to workplace */
    int i = reset_workplace(c, fixed, workplace);

    /* Set the free variables to the best fit */
    int for_pos = 0;
    int for_neg = 1;
    if(c->maximize) {
        for_pos = 1;
        for_neg = 0;
    }

    for(; i < c->num_vars; i++) {
        int n = c->function[i];
        if(n > 0) {
            workplace[i] = for_pos;
        } else if(n < 0) {
            workplace[i] = for_neg;
        } else {
            workplace[i] = 0;
        }
    }

    /* Calculate the best fit */
    return dot_product(c->function, workplace, c->num_vars);
}

bool check_restrictions(bip_context* c, int* vars)
{
    if(c->restrictions == NULL) {
        return true;
    }

    bool fact = true;

    for(int i = 0; fact && (i < c->num_rest); i++) {

        int* rests = c->restrictions->data[i];
        int type = rests[c->num_vars];
        int equl = rests[c->num_vars + 1];

        int real = dot_product(rests, vars, c->num_vars);

        if(type == GE) {
            fact = fact && (real >= equl);
            imp_node_log_calc(c, rests, vars, fact, i); /* LOG */
            continue;
        }

        if(type == LE) {
            fact = fact && (real <= equl);
            imp_node_log_calc(c, rests, vars, fact, i); /* LOG */
            continue;
        }

        fact = fact && (real == equl);
        imp_node_log_calc(c, rests, vars, fact, i); /* LOG */
    }

    return fact;
}

bool check_future_fact(bip_context* c, int* fixed, int* workplace)
{
    if(c->restrictions == NULL) {
        return true;
    }

    bool fact = true;
    for(int i = 0; fact && (i < c->num_rest); i++) {

        /* Flush fixed to workplace */
        int j = reset_workplace(c, fixed, workplace);

        int* rests = c->restrictions->data[i];
        int type = rests[c->num_vars];
        int equl = rests[c->num_vars + 1];

        /* Calculate margins */
        int top = INT_MAX;
        if((type == GE) || (type == EQ)) {

            /* Set free variables */
            for(int k = j; k < c->num_vars; k++) {
                int n = rests[k];
                if(n > 0) {
                    workplace[k] = 1;
                } else if(n < 0) {
                    workplace[k] = 0;
                } else {
                    workplace[k] = 0;
                }
            }

            /* Calculate scalar product */
            top = dot_product(rests, workplace, c->num_vars);
        }

        int bottom = INT_MIN;
        if((type == LE) || (type == EQ)) {

            /* Set free variables */
            for(int k = j; k < c->num_vars; k++) {
                int n = rests[k];
                if(n > 0) {
                    workplace[k] = 0;
                } else if(n < 0) {
                    workplace[k] = 1;
                } else {
                    workplace[k] = 0;
                }
            }

            /* Calculate scalar product */
            bottom = dot_product(rests, workplace, c->num_vars);
        }

        fact = fact && (bottom <= equl) && (equl <= top);
        imp_node_log_calc(c, rests, workplace, fact, i); /* LOG */
    }

    return fact;
}

