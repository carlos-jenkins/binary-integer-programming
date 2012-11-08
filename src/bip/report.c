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

#include "report.h"
#include "format.h"

bool implicit_report(bip_context* c)
{
    /* Create report file */
    FILE* report = fopen("reports/implicit.tex", "w");
    if(report == NULL) {
        return false;
    }

    /* Write preamble */
    bool success = insert_file("latex/header.tex", report);
    if(!success) {
        return false;
    }
    fprintf(report, "\n");

    /* Write header */
    fprintf(report, "\\section{%s}\n\n", "Implicit Enumeration");
    fprintf(report, "\\noindent{\\huge %s.} \\\\[0.4cm]\n",
                    "Binary Integer Programming");
    fprintf(report, "{\\LARGE %s.}\\\\[0.4cm]\n", "Operation Research");
    fprintf(report, "\\HRule \\\\[0.5cm]\n");

    /* Write description */
    fprintf(report, "\\indent ");
    success = insert_file("latex/preamble.tex", report);
    if(!success) {
        return false;
    }

    /* Write first section */
    fprintf(report, "\\\\[0.5cm]\n\\noindent{\\Large Details:}\n");
    fprintf(report, "\\begin{compactitem}\n");
    fprintf(report, "\\item %s : \\textsc{%s}. \n",
                    "Executed on", get_current_time());
    fprintf(report, "\\item %s : \\textsc{%lf %s}. \n",
                    "Execution time", c->execution_time,
                    "seconds");
    fprintf(report, "\\item %s : \\textsc{%i %s}. \n",
                    "Memory required", c->memory_required,
                    "bytes");
    fprintf(report, "\\end{compactitem}\n");
    fprintf(report, "\n");

    /* TOC */
    fprintf(report, "\\newpage\n\\tableofcontents\n\\newpage\n");
    fprintf(report, "\n");

    /* Write execution */
    success = copy_streams(c->report_buffer, report);
    if(!success) {
        return false;
    }

    /* End document */
    fprintf(report, "\\end{document}\n");
    fprintf(report, "\n");

    /* Save & swap buffers */
    int success_file = fflush(report);
    if(success_file == EOF) {
        return false;
    }
    success_file = fclose(c->report_buffer);
    if(success_file == EOF) {
        return false;
    }
    c->report_buffer = report;

    return true;
}

void imp_node_open(bip_context* c, int* vars, int* parents, int num)
{
    FILE* report = c->report_buffer;

    fprintf(report, "\\subsection{%s %i}\n", "Subproblem", num);

    bool branch = draw_branch(vars, parents, c->num_vars, num);

    fprintf(report, "\\begin{minipage}[t]{0.25\\textwidth}\n");
    if(branch) {
        fprintf(report, "    \\includegraphics[width=\\textwidth]"
                        "{reports/branch%i.pdf}\n", num);
    } else {
        fprintf(report, "    ERROR: Unable to generate branch %i.\n", num);
    }
    fprintf(report, "\\vfill\n");
    fprintf(report, "\\end{minipage}\n");
    fprintf(report, "\\begin{minipage}[t]{0.74\\textwidth}\n");
    fprintf(report, "\\lipsum[3]\n");
    fprintf(report, "\\vfill\n");
    fprintf(report, "\\end{minipage}\n");

}

void imp_node_close(bip_context* c, enum CloseReason reason)
{
    FILE* report = c->report_buffer;
    fprintf(report, "\\newpage\n");
    fprintf(report, "\n");
}

void imp_node_log_bf(bip_context* c, int bf, int alpha)
{
}

const char* GRAPH_HEADER = "digraph branch {\n"
"\n"
"    node [shape = circle, fixedsize = true];\n"
"    graph [ordering = out, splines = false];\n"
"    edge [labelfloat = true];\n"
"\n";
const char* LINK_TPL = "    %i -> %i "
"[label = <  <font point-size=\"20\" color=\"%s\">%s</font>"
"<font point-size=\"9\">%i</font> = %i>];\n";

bool draw_branch(int* vars, int* parents, int bounds, int num)
{
    /* Create branch file */
    char* path = g_strdup_printf("reports/branch%i.gv", num);
    FILE* branch = fopen(path, "w");
    g_free(path);
    if(branch == NULL) {
        return false;
    }

    /* Preamble */
    fprintf(branch, "%s", GRAPH_HEADER);

    /* Count levels */
    int levels = 0;
    for(int i = 0; i < bounds; i++) {
        if(vars[i] == -1) {
            break;
        }
        levels++;
    }

    /* Create a dummy node for each level */
    for(int i = 0; i < levels; i++) {
        char* dummy = g_strdup_printf(
                "    d%i [label = \"\", shape = none];\n", i + 1
            );
        fprintf(branch, "%s", dummy);
        g_free(dummy);
    }

    /* Create styled node for current node */
    char* current = g_strdup_printf(
            "    %i [style = bold, color = red];\n\n", num
        );
    fprintf(branch, "%s", current);
    g_free(current);

    /* Create links */
    for(int i = 0; i < levels; i++) {

        int left = vars[i] == 0;

        if(!left) {
            fprintf(branch,
                    "    %i -> d%i [color = white];\n", parents[i], i + 1
                );
        }

        fprintf(branch, LINK_TPL,
                parents[i],
                parents[i + 1],
                VAR_COLORS[i % VARS],
                VAR_NAMES[i % VARS],
                ((i / VARS) + 1),
                vars[i]
            );

        if(left) {
            fprintf(branch,
                    "    %i -> d%i [color = white];\n", parents[i], i + 1
                );
        }
    }

    /* Close file */
    fprintf(branch, "}\n");
    fclose(branch);

    /* Render graph */
    char* name = g_strdup_printf("branch%i", num);
    gv2pdf(name, "reports");
    g_free(name);

    return true;
}

