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
GtkSpinButton* vars;

GtkRadioButton* function_max;
GtkRadioButton* function_min;

GtkTreeView* function_view;
GtkTreeView* restrictions_view;

GtkFileChooser* load_dialog;
GtkFileChooser* save_dialog;

/* Functions */
void add_row(GtkToolButton *toolbutton, gpointer user_data);
void remove_row(GtkToolButton *toolbutton, gpointer user_data);

void edit_started_cb(GtkCellRenderer* renderer, GtkCellEditable* editable,
                     gchar* path, gpointer user_data);
void vars_changed(GtkSpinButton* spinbutton, gpointer user_data);
void change_vars(int vars);
bool change_function(int vars);
bool change_restrictions(int vars);

void process(GtkButton* button, gpointer user_data);

void save_cb(GtkButton* button, gpointer user_data);
void load_cb(GtkButton* button, gpointer user_data);
void save(FILE* file);
void load(FILE* file);

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
    vars = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "vars"));

    function_max = GTK_RADIO_BUTTON(
        gtk_builder_get_object(builder, "function_max"));
    function_min = GTK_RADIO_BUTTON(
        gtk_builder_get_object(builder, "function_min"));

    function_view = GTK_TREE_VIEW(
        gtk_builder_get_object(builder, "function_view"));
    restrictions_view = GTK_TREE_VIEW(
        gtk_builder_get_object(builder, "restrictions_view"));

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
    /* FIXMEFIXME */

    g_object_unref(G_OBJECT(builder));
    gtk_widget_show(GTK_WIDGET(window));
    gtk_main();

    return(0);
}

void edit_started_cb(GtkCellRenderer* renderer, GtkCellEditable* editable,
                     gchar* path, gpointer user_data)
{
    GtkAdjustment* adj;

    g_object_get(renderer, "adjustment", &adj, NULL);
    if(adj) {
        g_object_unref(adj);
    }

    adj = gtk_adjustment_new(
                    1.00,           /* the initial value. */
                    1.00,           /* the minimum value. */
                    10000000.00,    /* the maximum value. */
                    1.0,            /* the step increment. */
                    10.0,           /* the page increment. */
                    0.0             /* the page size. */
                );
    g_object_set(renderer, "adjustment", adj, NULL);
}

void vars_changed(GtkSpinButton* spinbutton, gpointer user_data)
{
    int vars = gtk_spin_button_get_value_as_int(spinbutton);
    change_vars(vars);
}

void change_vars(int vars)
{

    if(vars < 2) {
        show_error(window, "You need to define at least 2 variables. Sorry.");
        return;
    }
    bool changed1 = change_function(vars);
    bool changed2 = change_restrictions(vars);

    if(!changed1 || !changed2) {
        show_error(window,
            "Unable to allocated memory for this problem. Sorry.");
    }
}

bool change_function(int vars)
{
    /* Clear model */
    gtk_list_store_clear(
        GTK_LIST_STORE(gtk_tree_view_get_model(function_view)));

    /* Create the dynamic types array */
    GType* types = (GType*) malloc(2 * vars * sizeof(GType));
    if(types == NULL) {
        return false;
    }

    /* Set type in the dynamic types array */
    for(int i = 0; i < vars; i++) {
        types[i] = G_TYPE_INT;               /* Coeffs */
        types[vars + i] = G_TYPE_STRING;     /* Text   */
    }

    /* Create and fill the new model */
    GtkListStore* function = gtk_list_store_newv(2 * vars, types);
    GtkTreeIter iter;

    GValue initi = G_VALUE_INIT;
    g_value_init(&initi, G_TYPE_INT);

    GValue inits = G_VALUE_INIT;
    g_value_init(&inits, G_TYPE_STRING);

    gtk_list_store_append(function, &iter);
    for(int i = 0; i < vars; i++) {

        g_value_set_int(&initi, 1);
        gtk_list_store_set_value(function, &iter, i, &initi);

        char* text = var_name(1, i, i != 0);
        g_value_set_string(&inits, text);
        gtk_list_store_set_value(function, &iter, vars + i, &inits);
        free(text);
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
        /* Configure cell */
        GtkCellRenderer* cell = gtk_cell_renderer_spin_new();
        g_object_set(cell, "editable", true, NULL);
        g_signal_connect(G_OBJECT(cell),
                         "editing-started", G_CALLBACK(edit_started_cb),
                         GINT_TO_POINTER(i));
        //g_signal_connect(G_OBJECT(cell),
                         //"edited", G_CALLBACK(function_edited_cb),
                         //GINT_TO_POINTER(i));
        edit_started_cb(cell, NULL, NULL, NULL);
        /* Configure column */
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                                        "", cell,  /* Title, renderer */
                                        "markup", vars + i,
                                        NULL);
        gtk_tree_view_append_column(function_view, column);
    }

    /* Set the new model */
    gtk_tree_view_set_model(function_view, GTK_TREE_MODEL(function));

    /* Free resources */
    g_object_unref(G_OBJECT(function));
    free(types);

    return true;
}

bool change_restrictions(int vars)
{
    /* Clear model */
    gtk_list_store_clear(
        GTK_LIST_STORE(gtk_tree_view_get_model(restrictions_view)));

    return true;
}

void add_row(GtkToolButton *toolbutton, gpointer user_data)
{
    GtkListStore* restrictions =
        GTK_LIST_STORE(gtk_tree_view_get_model(restrictions_view));
    int rows = gtk_tree_model_iter_n_children(
                                    GTK_TREE_MODEL(restrictions), NULL);

    GtkTreeIter iter;
    gtk_list_store_append(restrictions, &iter);
    printf("TODO: Implement add_row()\n");
    //gtk_list_store_set(restrictions, &iter,
                        //0, sequence_name(rows),
                        //1, 1,
                        //2, 1,
                        //3, 1,
                        //4, "1",
                        //-1);
    //FIXMEFIXME
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
    int rows = gtk_tree_model_iter_n_children(
                                    GTK_TREE_MODEL(restrictions), NULL);
    if(rows < 3) {
        return;
    }

    GtkTreeSelection* selection = gtk_tree_view_get_selection(restrictions_view);
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

void process(GtkButton* button, gpointer user_data)
{
    /* FIXMEFIXME */
    printf("TODO: Implement process()\n");
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
    printf("Selected file: %s\n", filename);
    if((filename == NULL) || is_empty_string(filename)) {
        show_error(window, "Please select a file.");
        g_free(filename);
        save_cb(button, user_data);
        return;
    }
    gtk_widget_hide(GTK_WIDGET(save_dialog));

    /* Check extension */
    if(!g_str_has_suffix(filename, ".floyd")) {
        char* new_filename = g_strdup_printf("%s.floyd", filename);
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
    printf("Saving to file %s\n", filename);
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
    printf("Selected file: %s\n", filename);
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
    printf("Loading file %s\n", filename);
    load(file);

    /* Free resources */
    fclose(file);
    g_free(filename);
}

void save(FILE* file)
{
    printf("TODO: Implement save()\n");
}

void load(FILE* file)
{
    printf("TODO: Implement load()\n");
}

