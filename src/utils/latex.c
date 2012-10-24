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

#include "latex.h"

int latex2pdf(char* name, char* dir)
{
    char buffer[2000];

    /* Remove pdf if exists */
    sprintf(buffer, "%s/%s.pdf", dir, name);
    if(file_exists(buffer)) {
        sprintf(buffer, "rm %s/%s.pdf", dir, name);
        system(buffer);
    }

    /* Execute tex-pdf conversion if file is available */
    sprintf(buffer, "%s/%s.tex", dir, name);
    if(!file_exists(buffer)) {
        return -1;
    }
    sprintf(buffer, "pdflatex -halt-on-error -interaction batchmode "
                    "-output-directory %s %s/%s.tex > /dev/null",
                    dir, dir, name);
    int pdflatex_status = -1;
    for(int t = 0; t < 2; t++) { /* Double compilation */
        pdflatex_status = system(buffer);
    }
    if(pdflatex_status != 0) {
        return -2;
    }

    /* Cleanup */
    //sprintf(buffer, "rm %s/%s.tex", dir, name);
    //system(buffer);

    sprintf(buffer, "rm %s/%s.toc", dir, name);
    system(buffer);

    sprintf(buffer, "rm %s/%s.log", dir, name);
    system(buffer);

    sprintf(buffer, "rm %s/%s.aux", dir, name);
    system(buffer);

    /* Open */
    sprintf(buffer, "%s/%s.pdf", dir, name);
    if(!file_exists(buffer)) {
        return -3;
    }
    sprintf(buffer, "xdg-open %s/%s.pdf", dir, name);
    system(buffer);

    return 0;
}
