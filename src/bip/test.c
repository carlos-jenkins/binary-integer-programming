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
#include "latex.h"

int main(int argc, char **argv)
{
    printf("Testing binary integer programming algorythms...\n\n");

    /* Create context */
    bip_context* c = bip_context_new(10000, 10000); /* FIXME */
    if(c == NULL) {
        printf("ERROR: Unable to bip context... exiting.\n");
        return(-1);
    }

    /* Fill context*/
    /* FIXME */

    /* Show table */
    printf("-----------------------------------\n");
    /* matrix_print(d); */

    /* Run algorithm */
    bool success = implicit_enumeration(c);
    if(!success) {
        printf("ERROR: Implicit enumeration was unable to complete... exiting.\n");
        return(-2);
    }

    /* Show tables */
    printf("-----------------------------------\n");
    /* matrix_print(d); */
    printf("-----------------------------------\n");
    /* matrix_print(p); */

    /* Generate report */
    bool report_created = implicit_report(c);
    if(!report_created) {
        printf("ERROR: Report could not be created.\n");
    } else {
        printf("Report created at reports/implicit.tex\n");

        int as_pdf = latex2pdf("implicit", "reports");
        if(as_pdf == 0) {
            printf("PDF version available at reports/implicit.pdf\n");
        } else {
            printf("ERROR: Unable to convert report to PDF. Status: %i.\n",
                   as_pdf);
        }
    }

    /* Free resources */
    bip_context_free(c);
    return(0);
}
