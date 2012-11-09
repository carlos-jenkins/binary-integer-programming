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
    fprintf(report, "\\newpage\n");
    fprintf(report, "\\tableofcontents\n");
    fprintf(report, "\\newpage\n");
    fprintf(report, "\n");

    /* Model */
    fprintf(report, "\\section{%s}\n", "Model");
    fprintf(report, "\n");
    imp_model(report, c);
    fprintf(report, "\\end{adjustwidth}\n");
    fprintf(report, "\\newpage\n");
    fprintf(report, "\n");

    /* Write execution */
    fprintf(report, "\\section{%s}\n", "Resolution");
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

void imp_model(FILE* report, bip_context* c)
{
    /* Objective function */
    fprintf(report, "\\noindent\n");
    if(c->maximize) {
        fprintf(report, "{\\Large %s:}\n", "Maximize");
    } else {
        fprintf(report, "{\\Large %s:}\n", "Minimize");
    }
    fprintf(report, "{\\Large\n");
    fprintf(report, "\\begin{align*}\n");
    fprintf(report, "    Z = ");
    for(int i = 0; i < c->num_vars; i++) {

        int coeff = c->function[i];

        if(coeff == 0) {
            continue;
        }

        if(coeff < 0) {
            fprintf(report, " \\cred{-} ");
        } else if(i > 0) {
            fprintf(report, " \\cgreen{+} ");
        }
        fprintf(report, "%i\\textcolor{%s}{%s_%i}",
                        abs(coeff),
                        VAR_NAMES[i % VARS],
                        VAR_NAMES[i % VARS],
                        ((i / VARS) + 1)
                );
    }
    fprintf(report, "\n");
    fprintf(report, "\\end{align*}\n");
    fprintf(report, "}\n");
    fprintf(report, "\n");

    /* Restrictions */
    fprintf(report, "\\noindent\n");
    fprintf(report, "{\\Large %s:}\n", "Subject to");
    fprintf(report, "{\\Large\\[\n");
    fprintf(report, "\\begin{matrix}\n");
    for(int i = 0; i < c->num_rest; i++) {
        for(int j = 0; j < c->num_vars; j++) {

            int coeff = c->restrictions->data[i][j];

            if(coeff == 0) {
                fprintf(report, " & & ");
                continue;
            }

            if(coeff < 0) {
                fprintf(report, " \\cred{-}&");
            } else if(j > 0) {
                fprintf(report, " \\cgreen{+}&");
            } else {
                fprintf(report, " & ");
            }
            fprintf(report, " %i\\textcolor{%s}{%s_%i}&",
                            abs(coeff),
                            VAR_NAMES[j % VARS],
                            VAR_NAMES[j % VARS],
                            ((j / VARS) + 1)
                    );
        }
        int type = c->restrictions->data[i][c->num_vars];
        int equl = c->restrictions->data[i][c->num_vars + 1];
        if(type == LE) {
            fprintf(report, " \\le& ");
        } else if(type == GE) {
            fprintf(report, " \\ge& ");
        } else {
            fprintf(report, " =& ");
        }

        if(equl < 0) {
                fprintf(report, "\\cred{-}");
        } else {
            fprintf(report, "\\cgreen{+}");
        }
        fprintf(report, "%i", abs(equl));
        if(i < c->num_rest - 1) {
            fprintf(report, " \\\\");
        }
        fprintf(report, "\n");
    }
    fprintf(report, "\\end{matrix}\n");
    fprintf(report, "\\]}\n");
    fprintf(report, "\n");
}

void imp_node_open(bip_context* c, int* vars, int* parents, int num)
{
    FILE* report = c->report_buffer;

    fprintf(report, "\\subsection{%s %i}\n", "Subproblem", num);

    bool branch = draw_branch(vars, parents, c->num_vars, num);

    if(branch) {
        fprintf(report, "\\marginpar{%%\n");
        fprintf(report, "    \\vspace{0.6cm}\n");
        fprintf(report, "    \\includegraphics[width=\\marginparwidth]"
                        "{reports/branch%i.pdf}\n", num);
        fprintf(report, "    \\captionof{figure}{Subproblem %i branch.}\n",num);
        fprintf(report, "}\n");
    } else {
        fprintf(report,
            "\\marginpar{ERROR: Unable to generate branch %i}\n", num);
    }
    fprintf(report, "\n");
}

void imp_node_close(bip_context* c, enum CloseReason reason)
{
    FILE* report = c->report_buffer;
    fprintf(report, "\\begin{center}{\\Huge\n");
    switch(reason) {
        case doesnt_improve:
            fprintf(report, "\\cred{%s:\\\\\n\\vspace{0.3cm}\n%s.}\n",
                            "Close node", "Doesn't improve performance");
            break;
        case new_candidate:
            fprintf(report, "\\cgreen{%s:\\\\\n\\vspace{0.3cm}\n%s.}\n",
                            "Close node", "New candidate");
            break;
        case not_factible:
            fprintf(report, "\\cred{%s:\\\\\n\\vspace{0.3cm}\n%s.}\n",
                            "Close node", "No future factibility");
            break;
        default: /* expand */
            fprintf(report, "\\cgreen{%s:\\\\\n\\vspace{0.3cm}\n%s.}\n",
                            "Expand node", "Future factibility");
    }
    fprintf(report, "}\\end{center}\n");
    fprintf(report, "\\newpage\n");
    fprintf(report, "\n");
}

void imp_node_log_bf(bip_context* c, int* fixed, int* vars, int bf, int alpha)
{
    FILE* report = c->report_buffer;

    fprintf(report, "%s", "Considering solution: ");
    for(int i = 0; i < c->num_vars; i++) {
        int coeff = c->function[i];
        if(coeff == 0) {
            continue;
        }
        fprintf(report, "$\\textcolor{%s}{%s_%i} = %i$",
                VAR_NAMES[i % VARS],
                VAR_NAMES[i % VARS],
                ((i / VARS) + 1),
                vars[i]
        );
        if(i == c->num_vars - 1) {
            fprintf(report, ".");
        } else {
            fprintf(report, ", ");
        }
    }
    fprintf(report, "\n");
    fprintf(report, "\n");

    fprintf(report, "\\noindent\n");
    fprintf(report, "{\\Large %s:}\n", "Best fit");
    fprintf(report, "\\begin{align*}\n");
    fprintf(report, "    Z = ");

    int pos = 0;
    int count = 0;
    for(int i = 0; i < c->num_vars; i++) {
        int coeff = c->function[i];
        int fix = fixed[i];
        if(fix == -1) {
            break;
        }
        if((coeff == 0) || (fix == 0)) {
            pos++;
            continue;
        }
        fprintf(report, "%i\\cred{%s_%i}",
                        coeff, VAR_NAMES[i % VARS], ((i / VARS) + 1)
                );
        pos++;
        count++;
    }

    for(int i = pos; i < c->num_vars; i++) {
        int coeff = c->function[i];
        int flt = vars[i];
        if((coeff == 0) || (flt == 0)) {
            continue;
        }
        fprintf(report, "%i\\cgreen{%s_%i}",
                        coeff,
                        VAR_NAMES[i % VARS],
                        ((i / VARS) + 1)
                );
        count++;
    }

    if(count > 0) {
        fprintf(report, " =");
    }
    fprintf(report, " %i", bf);

    fprintf(report, "\\end{align*}\n");
    fprintf(report, "\n");

    bool free_alpha = true;
    char* alpha_txt = g_strdup_printf("%i", alpha);

    if(alpha == INT_MAX) {
        g_free(alpha_txt);
        free_alpha = false;
        alpha_txt = "+\\infty";
    } else if(alpha == INT_MIN) {
        g_free(alpha_txt);
        free_alpha = false;
        alpha_txt = "-\\infty";
    }
    fprintf(report, "Current $\\alpha$: \\textbf{$%s$}\n", alpha_txt);
    fprintf(report, "\n");

    if(c->maximize) {
        if(bf <= alpha) {
            fprintf(report, "$%i \\le %s \\longrightarrow$ %s.\n", bf, alpha_txt,
                            "Doesn't improve performance (maximizing)");
        } else {
            fprintf(report, "$%i > %s \\longrightarrow$ %s.\n", bf, alpha_txt,
                            "Improves performance (maximizing)");
        }

    } else {
        if(bf >= alpha) {
            fprintf(report, "$%i \\le %s \\longrightarrow$ %s.\n", bf, alpha_txt,
                            "Doesn't improve performance (minimizing)");
        } else {
            fprintf(report, "$%i < %s \\longrightarrow$ %s.\n", bf, alpha_txt,
                            "Improves performance (minimizing)");
        }
    }

    if(free_alpha) {
        g_free(alpha_txt);
    }
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

