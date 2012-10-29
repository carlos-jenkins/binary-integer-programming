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
#include <gtk/gtk.h>

/* GUI */
GtkWindow* window;
GtkSpinButton* vars;

GtkRadioButton* function_max;
GtkRadioButton* function_min;

GtkTreeView* function_view;
GtkListStore* function;

GtkTreeView* restrictions_view;
GtkListStore* restrictions;

GtkFileChooser* load_dialog;
GtkFileChooser* save_dialog;

/* Functions */
void add_row(GtkToolButton *toolbutton, gpointer user_data);
void remove_row(GtkToolButton *toolbutton, gpointer user_data);

void vars_changed(GtkSpinButton* spinbutton, gpointer user_data);

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
    function = GTK_LIST_STORE(
        gtk_builder_get_object(builder, "function"));

    restrictions_view = GTK_TREE_VIEW(
        gtk_builder_get_object(builder, "function_view"));
    restrictions = GTK_LIST_STORE(
        gtk_builder_get_object(builder, "restrictions"));

    load_dialog = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "load_dialog"));
    save_dialog = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "save_dialog"));

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

void vars_changed(GtkSpinButton* spinbutton, gpointer user_data) {
    printf("TODO: Implement vars_changed()\n");
}

void add_row(GtkToolButton *toolbutton, gpointer user_data)
{
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

