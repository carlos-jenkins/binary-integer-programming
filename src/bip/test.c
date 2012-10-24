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

#include "floyd.h"
#include "latex.h"

int main(int argc, char **argv)
{
    printf("Testing Floyd algorithm...\n\n");

    /* Create context */
    floyd_context* c = floyd_context_new(6);
    if(c == NULL) {
        printf("ERROR: Unable to create floyd context... exiting.\n");
        return(-1);
    }

    /* Fill context
     * | \ |   0 |   1 |   2 |   3 |   4 |   5 |
     * | 0 |   0 | inf | inf | inf |   5 |  11 |
     * | 1 |  16 |   0 |   6 |   1 | inf |   4 |
     * | 2 | inf |   7 |   0 |  12 | inf | inf |
     * | 3 | inf | inf |  19 |   0 |   9 | inf |
     * | 4 |   2 | inf |   8 | inf |   0 | inf |
     * | 5 | inf | inf | inf | inf |   3 |   0 |
     */

    char** n = c->names;
    n[0] = "A";
    n[1] = "B";
    n[2] = "C";
    n[3] = "D";
    n[4] = "E";
    n[5] = "F";

    matrix* d = c->table_d;
    matrix* p = c->table_p;

    d->data[0][4] = 5.0;
    d->data[0][5] = 11.0;

    d->data[1][0] = 16.0;
    d->data[1][2] = 6.0;
    d->data[1][3] = 1.0;
    d->data[1][5] = 4.0;

    d->data[2][1] = 7.0;
    d->data[2][3] = 12.0;

    d->data[3][2] = 19.0;
    d->data[3][4] = 9.0;

    d->data[4][0] = 2.0;
    d->data[4][2] = 8.0;

    d->data[5][4] = 3.0;


    /* Show table */
    printf("-----------------------------------\n");
    matrix_print(d);

    /* Run algorithm */
    bool success = floyd(c);
    if(!success) {
        printf("ERROR: Floyd algorithm was unable to complete... exiting.\n");
        return(-2);
    }

    /* Show tables */
    printf("-----------------------------------\n");
    matrix_print(d);
    printf("-----------------------------------\n");
    matrix_print(p);

    /* Generate report */
    bool report_created = floyd_report(c);
    if(!report_created) {
        printf("ERROR: Report could not be created.\n");
    } else {
        printf("Report created at reports/floyd.tex\n");

        int as_pdf = latex2pdf("floyd", "reports");
        if(as_pdf == 0) {
            printf("PDF version available at reports/floyd.pdf\n");
        } else {
            printf("ERROR: Unable to convert report to PDF. Status: %i.\n",
                   as_pdf);
        }
    }

    /* Free resources */
    floyd_context_free(c);
    return(0);
}
