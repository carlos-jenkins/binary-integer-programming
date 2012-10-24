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

#include "graphviz.h"

int gv2pdf(char* name, char* dir)
{
    char buffer[2000];

    /* Remove pdf if exists */
    sprintf(buffer, "%s/%s.pdf", dir, name);
    if(file_exists(buffer)) {
        sprintf(buffer, "rm %s/%s.pdf", dir, name);
        system(buffer);
    }

    /* Execute gv-eps conversion if file is available */
    sprintf(buffer, "%s/%s.gv", dir, name);
    if(!file_exists(buffer)) {
        return -1;
    }
    sprintf(buffer, "dot -Tps2 -o%s/%s.eps %s/%s.gv",
                    dir, name, dir, name);
    printf("%s\n", buffer);
    int dot_status = system(buffer);
    if(dot_status != 0) {
        printf("ERROR: dot finished with status %i\n", dot_status);
        return -2;
    }

    /* Convert eps to pdf */
    sprintf(buffer, "epstopdf --outfile=%s/%s.pdf %s/%s.eps",
                    dir, name, dir, name);
    printf("%s\n", buffer);
    int epstopdf_status = system(buffer);
    if(epstopdf_status != 0) {
        printf("ERROR: epstopdf finished with status %i\n", dot_status);
        return -3;
    }

    /* Cleanup */
    sprintf(buffer, "rm %s/%s.eps", dir, name);
    system(buffer);

    return 0;
}

int gv2png(char* name, char* dir)
{
    char buffer[2000];

    /* Remove png if exists */
    sprintf(buffer, "%s/%s.png", dir, name);
    if(file_exists(buffer)) {
        sprintf(buffer, "rm %s/%s.png", dir, name);
        system(buffer);
    }

    /* Execute gv-png conversion if file is available */
    sprintf(buffer, "%s/%s.gv", dir, name);
    if(!file_exists(buffer)) {
        return -1;
    }
    sprintf(buffer, "dot -Tpng -o%s/%s.png %s/%s.gv",
                    dir, name, dir, name);
    printf("%s\n", buffer);
    int dot_status = system(buffer);
    if(dot_status != 0) {
        printf("ERROR: dot finished with status %i\n", dot_status);
        return -2;
    }

    return 0;
}
