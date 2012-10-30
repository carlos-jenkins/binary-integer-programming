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

#include "utils.h"
#include "bip.h"
#include "report.h"
#include "latex.h"
#include "dialogs.h"
#include "format.h"
#include <gtk/gtk.h>

/* GUI */
GtkWindow* window;
GtkSpinButton* variables;

GtkRadioButton* function_max;
GtkRadioButton* function_min;

GtkTreeView* function_view;
GtkTreeView* restrictions_view;

GtkTreeModel* types_model;

GtkFileChooser* load_dialog;
GtkFileChooser* save_dialog;

/* Functions */
/* Functions : Main */
void clear_liststore(GtkTreeView* view);
void vars_changed_cb(GtkSpinButton* spinbutton, gpointer user_data);
bool change_vars(int vars);

void writeback(GtkTreeModel* model, gchar* path, int vars, bool is_var,
                    gchar* new_text, gpointer user_data);

void edit_started(GtkCellRenderer* renderer, GtkTreeModel* model,
            GtkCellEditable* editable, gchar* path, gpointer user_data);

/* Functions : Function */
void function_edit_started_cb(GtkCellRenderer* renderer,
            GtkCellEditable* editable, gchar* path, gpointer user_data);
void function_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data);

bool change_function(int vars);

/* Functions : Restrictions */
void restrictions_edit_started_cb(GtkCellRenderer* renderer,
            GtkCellEditable* editable, gchar* path, gpointer user_data);
void restrictions_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data);
void num_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data);

void toggle_type(GtkTreeView* tree_view, GtkTreePath* path,
                 GtkTreeViewColumn* column, gpointer user_data);
void write_symbol(GtkTreeModel* model,
                 GtkTreeIter* iter, int symbol, int size);

bool change_restrictions(int vars);

void add_row(GtkToolButton *toolbutton, gpointer user_data);
void remove_row(GtkToolButton *toolbutton, gpointer user_data);

/* Functions : Actions */
void process(GtkButton* button, gpointer user_data);
void save_cb(GtkButton* button, gpointer user_data);
void load_cb(GtkButton* button, gpointer user_data);
void save(FILE* file);
void load(FILE* file);


/**************
 * MAIN
 **************/
int main(int argc, char **argv)
{
    GtkBuilder* builder;
    GError* error = NULL;

    /* Starts Gtk+ subsystem */
    gtk_init(&argc, &argv);

    /* Load GUI interface */
    builder = gtk_builder_new();
    if(!gtk_builder_add_from_file(builder, "gui/bip.glade", &error)) {
        if(error) {
            g_warning("%s", error->message);
            g_error_free(error);
        } else {
            g_warning("Unknown error.");
        }
        return(1);
    }

    /* Get pointers to objects */
    window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    variables = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "variables"));

    function_max = GTK_RADIO_BUTTON(
        gtk_builder_get_object(builder, "function_max"));
    function_min = GTK_RADIO_BUTTON(
        gtk_builder_get_object(builder, "function_min"));

    function_view = GTK_TREE_VIEW(
        gtk_builder_get_object(builder, "function_view"));
    restrictions_view = GTK_TREE_VIEW(
        gtk_builder_get_object(builder, "restrictions_view"));

    types_model = GTK_TREE_MODEL(
        gtk_builder_get_object(builder, "types_model"));

    load_dialog = GTK_FILE_CHOOSER(
        gtk_builder_get_object(builder, "load_dialog"));
    save_dialog = GTK_FILE_CHOOSER(
        gtk_builder_get_object(builder, "save_dialog"));

    /* Configure interface */
    GtkFileFilter* file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter, "Custom data file (*.bip)");
    gtk_file_filter_add_pattern(file_filter, "*.bip");
    gtk_file_chooser_add_filter(load_dialog, file_filter);
    gtk_file_chooser_add_filter(save_dialog, file_filter);

    /* Connect signals */
    gtk_builder_connect_signals(builder, NULL);

    /* Initialize interface */
    change_vars(2);

    g_object_unref(G_OBJECT(builder));
    gtk_widget_show(GTK_WIDGET(window));
    gtk_main();

    return(0);
}

void clear_liststore(GtkTreeView* view)
{
    GtkTreeModel* model = gtk_tree_view_get_model(view);
    if(model != NULL) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    }
}

void vars_changed_cb(GtkSpinButton* spinbutton, gpointer user_data)
{
    int vars = gtk_spin_button_get_value_as_int(spinbutton);
    change_vars(vars);
}

bool change_vars(int vars)
{

    if(vars < 2) {
        show_error(window, "You need to define at least 2 variables. Sorry.");
        return false;
    }
    bool changed1 = change_function(vars);
    bool changed2 = change_restrictions(vars);

    if(!changed1 || !changed2) {
        show_error(window,
            "Unable to allocated memory for this problem. Sorry.");
        return false;
    }
    return true;
}

void writeback(GtkTreeModel* model, gchar* path, int vars, bool is_var,
                    gchar* new_text, gpointer user_data)
{
    int column = GPOINTER_TO_INT(user_data);
    DEBUG("%s at (%i, %i)\n", new_text, atoi(path), column);

    /* Get the coefficient */
    int coeff = 0;
    if(!is_empty_string(new_text)) {
        char* end;
        coeff = (int) strtol(new_text, &end, 10);
        if(*end != '\0') { /* Conversion wasn't successful */
            DEBUG("Unable to parse %s\n", new_text);
            return;
        }
    }

    /* Get reference to model */
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path);

    /* Set the model */
    GValue gvali = G_VALUE_INIT;
    g_value_init(&gvali, G_TYPE_INT);

    GValue gvals = G_VALUE_INIT;
    g_value_init(&gvals, G_TYPE_STRING);

    /* Coeff */
    g_value_set_int(&gvali, coeff);
    gtk_list_store_set_value(
        GTK_LIST_STORE(model), &iter, column, &gvali);
    g_value_unset(&gvali);

    /* Text */
    g_value_set_string(&gvals, "");
    if(!is_var) {
        char* text = num_name(coeff, true);
        g_value_set_string(&gvals, text);
        free(text);
    } else if(coeff != 0) {
        char* text = var_name(coeff, column, column == 0);
        g_value_set_string(&gvals, text);
        free(text);
    }
    gtk_list_store_set_value(
        GTK_LIST_STORE(model), &iter, vars + column, &gvals);

    /* Sign */
    g_value_set_string(&gvals, "");
    if(is_var) {
        if((column != 0)  && (coeff != 0)) {
            if(coeff > 0) {
                g_value_set_string(&gvals, PLUS);
            } else {
                g_value_set_string(&gvals, MINUS);
            }
        }
    }
    gtk_list_store_set_value(
        GTK_LIST_STORE(model), &iter, 2 * vars + column, &gvals);
    g_value_unset(&gvals);
}

void edit_started(GtkCellRenderer* renderer, GtkTreeModel* model,
            GtkCellEditable* editable, gchar* path, gpointer user_data)
{
    /* Remove previous adjustment */
    GtkAdjustment* adj;
    g_object_get(renderer, "adjustment", &adj, NULL);
    if(adj) {
        g_object_unref(adj);
    }

    /* Get column and data */
    if(path != NULL) {
        /* Configure editable */
        gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(editable), true);
        gtk_widget_set_size_request(GTK_WIDGET(editable), 100, 38);
        gtk_widget_queue_resize(GTK_WIDGET(editable));

        /* Configure editable */
        int column = GPOINTER_TO_INT(user_data);
        GtkTreeIter iter;
        GValue gval = G_VALUE_INIT;
        gtk_tree_model_get_iter_from_string(model, &iter, path);
        gtk_tree_model_get_value(model, &iter, column, &gval);
        int init = g_value_get_int(&gval);
        //printf("Found %i at %i\n", init, column);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(editable), (double) init);
        g_value_unset(&gval);

    }

    adj = gtk_adjustment_new(
                    0.0,            /* the initial value. (This is ignored) */
                   -10000000.00,    /* the minimum value.  */
                    10000000.00,    /* the maximum value.  */
                    1.0,            /* the step increment. */
                    10.0,           /* the page increment. */
                    0.0             /* the page size. */
                );
    g_object_set(renderer, "adjustment", adj, NULL);
}

/**************
 * FUNCTION
 **************/
void function_edit_started_cb(GtkCellRenderer* renderer,
            GtkCellEditable* editable, gchar* path, gpointer user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(function_view);
    edit_started(renderer, model, editable, path, user_data);
}

void function_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(function_view);
    int vars = gtk_spin_button_get_value_as_int(variables);
    writeback(model, path, vars, true, new_text, user_data);
}

bool change_function(int vars)
{
    /* Clear model */
    clear_liststore(function_view);

    /* Create the dynamic types array */
    GType* types = (GType*) malloc(3 * vars * sizeof(GType));
    if(types == NULL) {
        return false;
    }

    /* Set type in the dynamic types array */
    for(int i = 0; i < vars; i++) {
        types[i] = G_TYPE_INT;               /* Coeffs */
        types[vars + i] = G_TYPE_STRING;     /* Text   */
        types[2 * vars + i] = G_TYPE_STRING; /* Signs  */
    }

    /* Create and fill the new model */
    GtkListStore* function = gtk_list_store_newv(3 * vars, types);
    GtkTreeIter iter;

    GValue initi = G_VALUE_INIT;
    g_value_init(&initi, G_TYPE_INT);

    GValue inits = G_VALUE_INIT;
    g_value_init(&inits, G_TYPE_STRING);

    gtk_list_store_append(function, &iter);
    for(int i = 0; i < vars; i++) {

        g_value_set_int(&initi, 1);
        gtk_list_store_set_value(function, &iter, i, &initi);

        char* text = var_name(1, i, false);
        g_value_set_string(&inits, text);
        gtk_list_store_set_value(function, &iter, vars + i, &inits);
        free(text);

        g_value_set_string(&inits, PLUS);
        gtk_list_store_set_value(function, &iter, 2 * vars + i, &inits);
    }

    /* Clear the previous columns */
    for(int i = gtk_tree_view_get_n_columns(function_view) - 1; i >= 0; i--) {
        gtk_tree_view_remove_column(
                                function_view,
                                gtk_tree_view_get_column(function_view, i)
                            );
    }

    /* Create the new columns */
    for(int i = 0; i < vars; i++) {

        /* Create sign column */
        if(i > 0) {
            GtkCellRenderer* sign = gtk_cell_renderer_text_new();
            GtkTreeViewColumn* sign_c =
                gtk_tree_view_column_new_with_attributes(
                                            "", sign,  /* Title, renderer */
                                            "markup", 2 * vars + i,
                                            NULL);
            gtk_tree_view_append_column(function_view, sign_c);
        }

        /* Create text column */
        /* Create and configure cell */
        GtkCellRenderer* cell = gtk_cell_renderer_spin_new();

        gtk_cell_renderer_set_alignment(cell, 1.0, 0.5);
        g_object_set(cell, "editable", true, NULL);

        /* Configure callbacks */
        g_signal_connect(G_OBJECT(cell),
                    "editing-started", G_CALLBACK(function_edit_started_cb),
                    GINT_TO_POINTER(i));
        function_edit_started_cb(cell, NULL, NULL, GINT_TO_POINTER(i));

        g_signal_connect(G_OBJECT(cell),
                         "edited", G_CALLBACK(function_edited_cb),
                         GINT_TO_POINTER(i));

        /* Configure column */
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                                        "", cell,  /* Title, renderer */
                                        "markup", vars + i,
                                        NULL);
        gtk_tree_view_column_set_min_width(column, 100);
        gtk_tree_view_append_column(function_view, column);
    }
    gtk_tree_view_append_column(function_view, gtk_tree_view_column_new());

    /* Set the new model */
    gtk_tree_view_set_model(function_view, GTK_TREE_MODEL(function));

    /* Free resources */
    g_object_unref(G_OBJECT(function));
    free(types);

    return true;
}

/**************
 * RESTRICTIONS
 **************/
void restrictions_edit_started_cb(GtkCellRenderer* renderer,
            GtkCellEditable* editable, gchar* path, gpointer user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(restrictions_view);
    edit_started(renderer, model, editable, path, user_data);
}

void restrictions_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(restrictions_view);
    int vars = gtk_spin_button_get_value_as_int(variables) + 2;
    writeback(model, path, vars, true, new_text, user_data);
}

void num_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(restrictions_view);
    int vars = gtk_spin_button_get_value_as_int(variables) + 2;
    writeback(model, path, vars, false, new_text, user_data);
}

void toggle_type(GtkTreeView* tree_view, GtkTreePath* path,
                 GtkTreeViewColumn* column, gpointer user_data)
{
    /* Check for correct column */
    int vars = gtk_spin_button_get_value_as_int(variables);
    int size = vars + 2;
    GtkTreeViewColumn* dst =
        gtk_tree_view_get_column(restrictions_view, vars * 2 - 1);
    if(column != dst) {
        return;
    }

    /* Get reference */
    GtkTreeModel* model = gtk_tree_view_get_model(restrictions_view);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);

    /* Get previous value */
    GValue gval = G_VALUE_INIT;
    gtk_tree_model_get_value(model, &iter, size - 2, &gval);
    int old = g_value_get_int(&gval);
    g_value_unset(&gval);

    int symbol = LE;
    if(old == LE) {
        symbol = EQ;
    } else if(old == EQ) {
        symbol = GE;
    }
    write_symbol(model, &iter, symbol, size);
}

void write_symbol(GtkTreeModel* model, GtkTreeIter* iter, int symbol, int size)
{
    /* Set the next value */
    GValue initi = G_VALUE_INIT;
    g_value_init(&initi, G_TYPE_INT);

    GValue inits = G_VALUE_INIT;
    g_value_init(&inits, G_TYPE_STRING);

    if(symbol == LE) {
        g_value_set_int(&initi, LE);
        g_value_set_static_string(&inits, LES);
    } else if(symbol == EQ) {
        g_value_set_int(&initi, EQ);
        g_value_set_static_string(&inits, EQS);
    } else {
        g_value_set_int(&initi, GE);
        g_value_set_static_string(&inits, GES);
    }
    gtk_list_store_set_value(GTK_LIST_STORE(model), iter, size - 2, &initi);
    gtk_list_store_set_value(GTK_LIST_STORE(model), iter, 2*size - 2, &inits);
}

bool change_restrictions(int vars)
{
    /* Clear model */
    clear_liststore(restrictions_view);

    /* Create the dynamic types array */
    int size = vars + 2;
    GType* types = (GType*) malloc(3 * size * sizeof(GType));
    if(types == NULL) {
        return false;
    }

    /* Set type in the dynamic types array */
    for(int i = 0; i < size; i++) {
        types[i] = G_TYPE_INT;               /* Coeffs */
        types[size + i] = G_TYPE_STRING;     /* Text   */
        types[2 * size + i] = G_TYPE_STRING; /* Signs  */
    }

    /* Create and fill the new model */
    GtkListStore* restrictions = gtk_list_store_newv(3 * size, types);
    GtkTreeIter iter;

    GValue initi = G_VALUE_INIT;
    g_value_init(&initi, G_TYPE_INT);

    GValue inits = G_VALUE_INIT;
    g_value_init(&inits, G_TYPE_STRING);

    for(int j = 0; j < vars; j++) {

        /* Set the coefficients */
        gtk_list_store_append(restrictions, &iter);
        for(int i = 0; i < vars; i++) {

            int calc = 0;
            if(j == i) {
                calc = 1;
            }

            g_value_set_int(&initi, calc);
            gtk_list_store_set_value(restrictions, &iter, i, &initi);

            g_value_set_string(&inits, "");
            if(j == i) {
                char* text = var_name(calc, i, false);
                g_value_set_string(&inits, text);
                free(text);
            }
            gtk_list_store_set_value(restrictions, &iter, size + i, &inits);

            g_value_set_string(&inits, "");
            if(j == i) {
                g_value_set_string(&inits, PLUS);
            }
            gtk_list_store_set_value(restrictions, &iter, 2 * size + i, &inits);
        }

        /* Set type */
        g_value_set_int(&initi, GE);
        gtk_list_store_set_value(restrictions, &iter, size - 2, &initi);

        g_value_set_string(&inits, GES);
        gtk_list_store_set_value(restrictions, &iter, 2 * size - 2, &inits);

        g_value_set_string(&inits, "");
        gtk_list_store_set_value(restrictions, &iter, 3 * size - 2, &inits);

        /* Set equality */
        g_value_set_int(&initi, 0);
        gtk_list_store_set_value(restrictions, &iter, size - 1, &initi);

        char* text = num_name(0, false);
        g_value_set_string(&inits, text);
        gtk_list_store_set_value(restrictions, &iter, 2 * size - 1, &inits);
        free(text);

        g_value_set_string(&inits, "");
        gtk_list_store_set_value(restrictions, &iter, 3 * size - 1, &inits);

    }

    /* Clear the previous columns */
    for(int i = gtk_tree_view_get_n_columns(restrictions_view) - 1; i >= 0; i--) {
        gtk_tree_view_remove_column(
                                restrictions_view,
                                gtk_tree_view_get_column(restrictions_view, i)
                            );
    }

    /* Create the new columns */
    for(int i = 0; i < size; i++) {

        /* Especial column for type */
        if(i == (size - 2)) {
            GtkCellRenderer* type = gtk_cell_renderer_text_new();
            GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                                        "", type,  /* Title, renderer */
                                        "markup", size + i,
                                        NULL);
            gtk_tree_view_append_column(restrictions_view, column);
            continue;
        }

        /* Create sign column */
        if((i != 0) && (i != size - 1)) {
            GtkCellRenderer* sign = gtk_cell_renderer_text_new();
            GtkTreeViewColumn* sign_c =
                gtk_tree_view_column_new_with_attributes(
                                            "", sign,  /* Title, renderer */
                                            "markup", 2 * size + i,
                                            NULL);
            gtk_tree_view_append_column(restrictions_view, sign_c);
        }

        /* Create text column */
        /* Create and configure cell */
        GtkCellRenderer* cell = gtk_cell_renderer_spin_new();

        gtk_cell_renderer_set_alignment(cell, 1.0, 0.5);
        g_object_set(cell, "editable", true, NULL);

        /* Configure callbacks */
        g_signal_connect(G_OBJECT(cell),
                    "editing-started", G_CALLBACK(restrictions_edit_started_cb),
                    GINT_TO_POINTER(i));
        restrictions_edit_started_cb(cell, NULL, NULL, GINT_TO_POINTER(i));

        if(i < size - 2) {
            g_signal_connect(G_OBJECT(cell),
                         "edited", G_CALLBACK(restrictions_edited_cb),
                         GINT_TO_POINTER(i));
        } else {
            g_signal_connect(G_OBJECT(cell),
                         "edited", G_CALLBACK(num_edited_cb),
                         GINT_TO_POINTER(i));
        }

        /* Configure column */
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                                        "", cell,  /* Title, renderer */
                                        "markup", size + i,
                                        NULL);
        gtk_tree_view_column_set_min_width(column, 100);
        gtk_tree_view_append_column(restrictions_view, column);
    }
    gtk_tree_view_append_column(restrictions_view, gtk_tree_view_column_new());

    /* Set the new model */
    gtk_tree_view_set_model(restrictions_view, GTK_TREE_MODEL(restrictions));

    /* Free resources */
    g_object_unref(G_OBJECT(restrictions));
    free(types);

    return true;
}

void add_row(GtkToolButton* toolbutton, gpointer user_data)
{
    /* Context data */
    GtkListStore* restrictions =
        GTK_LIST_STORE(gtk_tree_view_get_model(restrictions_view));
    int vars = gtk_spin_button_get_value_as_int(variables);

    GValue initi = G_VALUE_INIT;
    g_value_init(&initi, G_TYPE_INT);

    GValue inits = G_VALUE_INIT;
    g_value_init(&inits, G_TYPE_STRING);

    GtkTreeIter iter;
    gtk_list_store_append(restrictions, &iter);

    int size = vars + 2;
    for(int i = 0; i < vars; i++) {

        g_value_set_int(&initi, 1);
        gtk_list_store_set_value(restrictions, &iter, i, &initi);

        char* text = var_name(1, i, false);
        g_value_set_string(&inits, text);
        gtk_list_store_set_value(restrictions, &iter, size + i, &inits);
        free(text);

        g_value_set_string(&inits, PLUS);
        gtk_list_store_set_value(restrictions, &iter, 2 * size + i, &inits);
    }
    /* Set type */
    g_value_set_int(&initi, GE);
    gtk_list_store_set_value(restrictions, &iter, size - 2, &initi);

    g_value_set_string(&inits, GES);
    gtk_list_store_set_value(restrictions, &iter, 2 * size - 2, &inits);

    g_value_set_string(&inits, "");
    gtk_list_store_set_value(restrictions, &iter, 3 * size - 2, &inits);

    /* Set equality */
    g_value_set_int(&initi, 0);
    gtk_list_store_set_value(restrictions, &iter, size - 1, &initi);

    char* text = num_name(0, false);
    g_value_set_string(&inits, text);
    gtk_list_store_set_value(restrictions, &iter, 2 * size - 1, &inits);
    free(text);

    g_value_set_string(&inits, "");
    gtk_list_store_set_value(restrictions, &iter, 3 * size - 1, &inits);

    /* Select new row */
    GtkTreePath* model_path = gtk_tree_model_get_path(
                                GTK_TREE_MODEL(restrictions), &iter);
    gtk_tree_view_set_cursor(restrictions_view, model_path,
                             gtk_tree_view_get_column(restrictions_view, 0),
                             true);
    gtk_tree_path_free(model_path);
    return;
}

void remove_row(GtkToolButton *toolbutton, gpointer user_data)
{
    GtkListStore* restrictions =
        GTK_LIST_STORE(gtk_tree_view_get_model(restrictions_view));
    int vars = gtk_spin_button_get_value_as_int(variables);
    int rows = gtk_tree_model_iter_n_children(
                                    GTK_TREE_MODEL(restrictions), NULL);
    if(rows <= vars) {
        return;
    }

    GtkTreeSelection* selection =
        gtk_tree_view_get_selection(restrictions_view);
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, NULL, &iter)) {

        bool valid = gtk_list_store_remove(restrictions, &iter);
        if(!valid) {
            valid = gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(restrictions),
                                                  &iter, NULL, rows - 1);
        }

        if(valid) {
            GtkTreePath* model_path = gtk_tree_model_get_path(
                                        GTK_TREE_MODEL(restrictions), &iter);
            gtk_tree_view_set_cursor(restrictions_view, model_path, NULL, false);
            gtk_tree_path_free(model_path);
        }
    }
}

/**************
 * ACTIONS
 **************/
void process(GtkButton* button, gpointer user_data)
{
    /* FIXMEFIXME */
    DEBUG("TODO: Implement process()\n");
}

void save_cb(GtkButton* button, gpointer user_data)
{
    /* Load save dialog */
    int response = gtk_dialog_run(GTK_DIALOG(save_dialog));
    if(response != 0) {
        gtk_widget_hide(GTK_WIDGET(save_dialog));
        return;
    }

    /* Get filename */
    char* filename;
    filename = gtk_file_chooser_get_filename(save_dialog);

    /* Check is not empty */
    DEBUG("Selected file: %s\n", filename);
    if((filename == NULL) || is_empty_string(filename)) {
        show_error(window, "Please select a file.");
        g_free(filename);
        save_cb(button, user_data);
        return;
    }
    gtk_widget_hide(GTK_WIDGET(save_dialog));

    /* Check extension */
    if(!g_str_has_suffix(filename, ".bip")) {
        char* new_filename = g_strdup_printf("%s.bip", filename);
        g_free(filename);
        filename = new_filename;
    }

    /* Try to open file for writing */
    FILE* file = fopen(filename, "w");
    if(file == NULL) {
        show_error(window, "An error ocurred while trying to open "
                           "the file. Check you have permissions.");
        g_free(filename);
        return;
    }

    /* Save current context */
    DEBUG("Saving to file %s\n", filename);
    save(file);

    /* Free resources */
    fclose(file);
    g_free(filename);
}

void load_cb(GtkButton* button, gpointer user_data)
{
    /* Load load dialog */
    int response = gtk_dialog_run(GTK_DIALOG(load_dialog));
    if(response != 0) {
        gtk_widget_hide(GTK_WIDGET(load_dialog));
        return;
    }

    /* Get filename */
    char* filename;
    filename = gtk_file_chooser_get_filename(load_dialog);

    /* Check is not empty */
    DEBUG("Selected file: %s\n", filename);
    if((filename == NULL) || is_empty_string(filename)) {
        show_error(window, "Please select a file.");
        g_free(filename);
        load_cb(button, user_data);
        return;
    }
    gtk_widget_hide(GTK_WIDGET(load_dialog));

    /* Try to open file for reading */
    if(!file_exists(filename)) {
        show_error(window, "The selected file doesn't exists.");
        return;
    }
    FILE* file = fopen(filename, "r");
    if(file == NULL) {
        show_error(window, "An error ocurred while trying to open "
                           "the file. Check you have permissions.");
        return;
    }

    /* Load file */
    DEBUG("Loading file %s\n", filename);
    load(file);

    /* Free resources */
    fclose(file);
    g_free(filename);
}

void save(FILE* file)
{
    /** Format:
     * 10                      : Number of variables.
     * 0/1                     : If the objetive function is to maximize.
     * 7 8 9 45 0 7 23 29      : Coefficients of the objective function.
     * 12                      : Number of restrictions.
     * 5 7 4 20 5 0 13 70 1 50 : Coefficients of the first restriction.
     * (....)                  : Many lines as restrictions.
     */

    /* Get information */
    int vars = gtk_spin_button_get_value_as_int(variables);
    GtkTreeModel* function = gtk_tree_view_get_model(function_view);
    bool is_max = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(function_max));

    GtkTreeModel* restrictions = gtk_tree_view_get_model(restrictions_view);
    int num_restrictions = gtk_tree_model_iter_n_children(restrictions, NULL);

    /* Model iteration */
    GtkTreeIter iter;
    gtk_tree_model_get_iter_first(function, &iter);
    GValue gval = G_VALUE_INIT;

    /* Write file according to format */
    fprintf(file, "%i\n", vars);
    fprintf(file, "%i\n", is_max);

    for(int i = 0; i < vars; i++) {
        gtk_tree_model_get_value(function, &iter, i, &gval);
        int val = g_value_get_int(&gval);
        g_value_unset(&gval);
        fprintf(file, "%i ", val);
    }
    fprintf(file, "\n");

    fprintf(file, "%i\n", num_restrictions);

    bool iter_set = gtk_tree_model_get_iter_first(restrictions, &iter);
    while(iter_set) {
        for(int i = 0; i < vars + 2; i++) {
            gtk_tree_model_get_value(restrictions, &iter, i, &gval);
            int val = g_value_get_int(&gval);
            g_value_unset(&gval);
            fprintf(file, "%i ", val);
        }
        fprintf(file, "\n");
        iter_set = gtk_tree_model_iter_next(restrictions, &iter);
    }
}

void load(FILE* file)
{
    /* Load number of variables */
    int vars = 0;
    fscanf(file, "%i%*c", &vars);
    bool success = change_vars(vars);
    if(!success) {
        return;
    }
    gtk_spin_button_set_value(variables, (gdouble)vars);

    /* Reference new models */
    GtkTreeModel* function = gtk_tree_view_get_model(function_view);
    GtkTreeModel* restrictions = gtk_tree_view_get_model(restrictions_view);

    /* Load if objetive function is to maximize */
    int is_max = 0;
    fscanf(file, "%i%*c", &is_max);
    if(is_max) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(function_max), true);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(function_min), true);
    }

    /* Load coefficients of the objetive function */
    int coeff = 0;
    char buff[15];
    for(int i = 0; i < vars; i++) {
        fscanf(file, "%i", &coeff);
        sprintf(buff, "%d", coeff);
        writeback(function, "0", vars, true, (gchar*)&buff, GINT_TO_POINTER(i));
    }
    fscanf(file, "%*c");

    /* Load number of restrictions */
    int num_restrictions = 0;
    fscanf(file, "%i%*c", &num_restrictions);
    for(int i = 0; i < (num_restrictions - vars); i++) {
        add_row(NULL, NULL);
    }

    /* Load coefficients of each restriction */
    int size = vars + 2;
    char buff2[15];
    GtkTreeIter iter;
    bool iter_set = gtk_tree_model_get_iter_first(restrictions, &iter);
    for(int i = 0; iter_set && (i < num_restrictions); i++) {
        for(int j = 0; j < size; j++) {
            /* Coefficient or num */
            if(j != (size - 2)) {
                fscanf(file, "%i", &coeff);
                sprintf(buff,  "%d", coeff); /* new_text */
                sprintf(buff2, "%d", i);     /* path */
                writeback(restrictions,
                    (gchar*)&buff2,
                    size, j < (size - 1),
                    (gchar*)&buff,
                    GINT_TO_POINTER(j));
            /* Equation type */
            } else {
                fscanf(file, "%i", &coeff);
                DEBUG("Found type to be %i at (%i, %i).\n", coeff, i, j);
                write_symbol(restrictions, &iter, coeff, size);
            }
        }
        fscanf(file, "%*c");

        iter_set = gtk_tree_model_iter_next(restrictions, &iter);
    }
}
