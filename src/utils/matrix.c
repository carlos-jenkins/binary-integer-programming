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

#include "matrix.h"

bool matrix_copy(matrix* src, matrix* dest)
{
    if((src == NULL) ||
       (dest == NULL) ||
       (src->rows != dest->rows) ||
       (src->columns != dest->columns)) {
        return false;
    }

    for(int i = 0; i < src->rows; i++) {
        for(int j = 0; j < src->columns; j++) {
            dest->data[i][j] = src->data[i][j];
        }
    }
    return true;
}

void matrix_fill(matrix* m, MATRIX_DATATYPE value)
{
    for(int i = 0; i < m->rows; i++) {
        for(int j = 0; j < m->columns; j++) {
            m->data[i][j] = value;
        }
    }
}

void matrix_print(matrix* m)
{
    printf("Table: %i x %i\n", m->rows, m->columns);
    for(int i = 0; i < m->rows; i++) {
        for(int j = 0; j < m->columns; j++) {
            int cell = m->data[i][j];
            if(cell == INT_MAX) {
                printf("+oo ");
            } else if(cell == INT_MIN) {
                printf("-oo ");
            } else {
                printf("%i ", cell);
            }
        }
        printf("\n");
    }
}

matrix* matrix_new(int rows, int columns, MATRIX_DATATYPE fill)
{
    /* Check if the matrix has a correct size */
    if(rows < 1 || columns < 1) {
        return NULL;
    }

    /* Allocate structure */
    matrix* m = (matrix*) malloc(sizeof(matrix));
    if(m == NULL) {
        return NULL;
    }

    m->rows = rows;
    m->columns = columns;
    m->data = NULL;

    /* Create the rows array with unknown columns pointers */
    m->data = (MATRIX_DATATYPE**) malloc(rows * sizeof(MATRIX_DATATYPE*));

    /* Check if allocation could be done */
    if(m->data == NULL) {
        free(m);
        return NULL;
    }

    /* Create the columns arrays and put their pointers on the rows array */
    for(int i = 0; i < rows; i++) {

        m->data[i] = (MATRIX_DATATYPE*) malloc(columns * sizeof(MATRIX_DATATYPE));

        /* Check if allocation could be done */
        if(m->data[i] == NULL) {

            /* Allocation failed, rollback and free memory */
            for(int j = (i - 1); j >= 0; j--) {
                free(m->data[j]);
            }

            /* Free the rows array */
            free(m->data);

            free(m);
            return NULL;
        }
    }

    /* Initialize the matrix */
    matrix_fill(m, fill);

    return m;
}

unsigned int matrix_sizeof(matrix* m)
{
    return (m->rows * sizeof(MATRIX_DATATYPE*)) +
           (m->rows * (m->columns * sizeof(MATRIX_DATATYPE)));
}

void matrix_free(matrix* m)
{
    /* Check if matrix has somethign */
    if((m != NULL) && (m->data != NULL)) {

        /* Release the columns arrays */
        for(int i = 0; i < m->rows; i++) {
            if(m->data[i] !=  NULL) {
                free(m->data[i]);
            } else {
                printf("Warning! Row %i was NULL!!", i);
            }
        }

        /* Release the rows array */
        free(m->data);
        m->data = NULL;
        free(m);
    }

    return;
}
