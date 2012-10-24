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

#include "implicit.h"
#include "latex.h"
#include "dialogs.h"
#include <gtk/gtk.h>

/* GUI */
GtkWindow* window;
GtkSpinButton* nodes;
GtkTreeView* input;
GtkImage* graph;

GtkFileChooser* load_dialog;
GtkFileChooser* save_dialog;

/* Context */
floyd_context* c = NULL;
matrix* adj_matrix = NULL;
char** names = NULL;

/* Functions */
bool change_matrix(int size);
void change_matrix_cb(GtkSpinButton* spinbutton, gpointer user_data);
void update_graph();
void cell_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data);
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
    if(!gtk_builder_add_from_file(builder, "gui/floyd.glade", &error)) {
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
    nodes = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "nodes"));
    input = GTK_TREE_VIEW(gtk_builder_get_object(builder, "input"));
    graph = GTK_IMAGE(gtk_builder_get_object(builder, "graph"));

    load_dialog = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "load_dialog"));
    save_dialog = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "save_dialog"));

    GtkFileFilter* file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter, "Custom data file (*.floyd)");
    gtk_file_filter_add_pattern(file_filter, "*.floyd");
    gtk_file_chooser_add_filter(load_dialog, file_filter);
    gtk_file_chooser_add_filter(save_dialog, file_filter);

    /* Connect signals */
    gtk_builder_connect_signals(builder, NULL);

    /* Initialize interface */
    change_matrix(2);

    g_object_unref(G_OBJECT(builder));
    gtk_widget_show(GTK_WIDGET(window));
    gtk_main();

    return(0);
}

bool change_matrix(int size)
{
    /* Validate input */
    if(size < 2) {
        return false;
    }

    int old_size = 0;
    if(adj_matrix != NULL) {
        old_size = adj_matrix->columns;
    }

    /* Try to create the adjacency matrix */
    if(adj_matrix != NULL) {
        matrix_free(adj_matrix);
    }
    adj_matrix = matrix_new(size, size, PLUS_INF);
    if(adj_matrix == NULL) {
        return false;
    }
    for(int i = 0; i < size; i++) {
        adj_matrix->data[i][i] = 0.0;
    }

    /* Try to create names array */
    if(names != NULL) {
        for(int i = 0; i < old_size; i++) {
            free(names[i]);
        }
        free(names);
    }
    names = (char**) malloc(size * sizeof(char*));
    if(names == NULL) {
        return false;
    }
    for(int i = 0; i < size; i++) {
        names[i] = sequence_name(i);
    }

    /* Create the dynamic types array */
    int rsize = size + 1;
    GType* types = (GType*) malloc(3 * rsize * sizeof(GType));
    if(types == NULL) {
        matrix_free(adj_matrix);
        floyd_context_free(c);
        return false;
    }
    for(int i = 0; i < rsize; i++) {
        types[i] = G_TYPE_STRING;
        types[rsize + i] = G_TYPE_BOOLEAN;     /* Editable */
        types[2 * rsize + i] = G_TYPE_BOOLEAN; /* Background set */
    }

    /* Create and fill the new model */
    GtkListStore* model = gtk_list_store_newv(3 * rsize, types);
    GtkTreeIter iter;

    GValue init = G_VALUE_INIT;
    g_value_init(&init, G_TYPE_STRING);

    GValue initb = G_VALUE_INIT;
    g_value_init(&initb, G_TYPE_BOOLEAN);

    for(int i = 0; i < rsize; i++) {
        gtk_list_store_append(model, &iter);

        /* First row */
        if(i == 0) {
            for(int j = 0; j < rsize; j++) {
                /* Set value */
                if(j > 0) {
                    g_value_set_string(&init, sequence_name(j - 1));
                    g_value_set_boolean(&initb, true);
                } else {
                    g_value_set_string(&init, "");
                    g_value_set_boolean(&initb, false);
                }
                gtk_list_store_set_value(model, &iter, j, &init);

                /* Set editable */
                gtk_list_store_set_value(model, &iter, rsize + j, &initb);

                /* Set background set */
                g_value_set_boolean(&initb, true);
                gtk_list_store_set_value(model, &iter, 2 * rsize + j, &initb);
            }
            continue;
        }

        /* Other rows, first cell */
        /* Set value */
        g_value_set_string(&init, sequence_name(i - 1));
        gtk_list_store_set_value(model, &iter, 0, &init);

        /* Set editable */
        g_value_set_boolean(&initb, false);
        gtk_list_store_set_value(model, &iter, rsize, &initb);

        /* Set background set */
        g_value_set_boolean(&initb, true);
        gtk_list_store_set_value(model, &iter, 2 * rsize, &initb);

        /* Other rows, other cells */
        for(int j = 1; j < rsize; j++) {

            /* Set value */
            if(i == j) {
                g_value_set_string(&init, "0");
            } else {
                g_value_set_string(&init, "oo");
            }
            gtk_list_store_set_value(model, &iter, j, &init);

            /* Set editable */
            g_value_set_boolean(&initb, i != j);
            gtk_list_store_set_value(model, &iter, rsize + j, &initb);

            /* Set background set */
            g_value_set_boolean(&initb, false);
            gtk_list_store_set_value(model, &iter, 2 * rsize + j, &initb);
        }
    }

    /* Clear the previous matrix */
    for(int i = gtk_tree_view_get_n_columns(input) - 1; i >= 0; i--) {
        gtk_tree_view_remove_column(input,
                                        gtk_tree_view_get_column(input, i)
                                    );
    }

    /* Create the matrix */
    for(int i = 0; i < rsize; i++) {
        /* Configure cell */
        GtkCellRenderer* cell = gtk_cell_renderer_text_new();
        g_object_set(cell, "cell-background", "Black", NULL);
        g_signal_connect(G_OBJECT(cell),
                         "edited", G_CALLBACK(cell_edited_cb),
                         GINT_TO_POINTER(i));
        /* Configure column */
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                                        "", cell,  /* Title, renderer */
                                        "text", i,
                                        "editable", rsize + i,
                                        "cell-background-set", 2 * rsize + i,
                                        NULL);
        gtk_tree_view_append_column(input, column);
    }
    GtkTreeViewColumn* close_column = gtk_tree_view_column_new();
    gtk_tree_view_append_column(input, close_column);

    /* Set the new model */
    gtk_tree_view_set_model(input, GTK_TREE_MODEL(model));

    /* Update the graph */
    update_graph();

    /* Free resources */
    g_object_unref(G_OBJECT(model));
    free(types);

    return true;
}

void update_graph()
{
    /* Create graph */
    floyd_graph(adj_matrix, names);
    if(gv2png("graph", "reports") < 0) {
        gtk_image_set_from_pixbuf(graph, NULL);
        return;
    }

    /* Load image */
    GError* error = NULL;
    //GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_size(
                            //"reports/graph.png", 300, 300, &error);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("reports/graph.png", &error);
    if(pixbuf == NULL) {
        if(error) {
            g_warning("%s", error->message);
            g_error_free(error);
        } else {
            g_warning("Unknown error while loading the graph.");
        }
        return;
    }
    gtk_image_set_from_pixbuf(graph, pixbuf);

    g_object_unref(pixbuf);
}

void cell_edited_cb(GtkCellRendererText* renderer, gchar* path,
                    gchar* new_text, gpointer user_data)
{
    int row = atoi(path);
    int column = GPOINTER_TO_INT(user_data);
    printf("%s at (%i, %i)\n", new_text, row, column);

    /* Validate row and column just in case */
    if((row == column) || column == 0) {
        return;
    }

    /* Get reference to model */
    GtkTreePath* model_path = gtk_tree_path_new_from_string(path);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(gtk_tree_view_get_model(input), &iter, model_path);
    gtk_tree_path_free(model_path);

    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);

    /* A node is being renamed */
    if(row == 0) {
        if(!is_empty_string(new_text)) {
            /* Set node at first row */
            g_value_set_string(&value, new_text);
            gtk_list_store_set_value(
                            GTK_LIST_STORE(gtk_tree_view_get_model(input)),
                            &iter, column, &value);

            /* Set node at first column */
            char* at_y = g_strdup_printf("%i", column);
            model_path = gtk_tree_path_new_from_string(at_y);
            gtk_tree_model_get_iter(
                    gtk_tree_view_get_model(input), &iter, model_path);
            gtk_tree_path_free(model_path);
            g_free(at_y);
            gtk_list_store_set_value(
                            GTK_LIST_STORE(gtk_tree_view_get_model(input)),
                            &iter, 0, &value);

            g_free(names[column - 1]);
            names[column - 1] = g_strdup(new_text);

            /* Update the graph */
            update_graph();
        }
        return;
    }

    /* A distance is being set */
    /* INFINITY */
    int is_inf = strncmp(new_text, "oo", 2);
    if(is_inf == 0 || is_empty_string(new_text)) {
        g_value_set_string(&value, "oo");
        gtk_list_store_set_value(
                            GTK_LIST_STORE(gtk_tree_view_get_model(input)),
                            &iter, column, &value);

        adj_matrix->data[row - 1][column - 1] = PLUS_INF;

        /* Update the graph */
        update_graph();
        return;
    }
    /* Distance */
    char* end;
    int distance = (int) strtol(new_text, &end, 10);
    if((end != new_text) && (*end == '\0') && (distance > 0)) {
        char* distance_as_string = g_strdup_printf("%i", distance);
        g_value_set_string(&value, distance_as_string);
        gtk_list_store_set_value(
                            GTK_LIST_STORE(gtk_tree_view_get_model(input)),
                            &iter, column, &value);
        g_free(distance_as_string);

        adj_matrix->data[row - 1][column - 1] = (float) distance;

        /* Update the graph */
        update_graph();
        return;
    }
    return;

}

void change_matrix_cb(GtkSpinButton* spinbutton, gpointer user_data)
{
    /* Get the number of requested nodes */
    int value = gtk_spin_button_get_value_as_int(spinbutton);
    bool success = change_matrix(value);
    if(!success) {
        show_error(window, "Unable to allocate enough memory for "
                           "this problem. Sorry.");
    }
    return;
}

void process(GtkButton* button, gpointer user_data)
{
    /* Try to create the new context */
    if(c != NULL) {
        floyd_context_free(c);
    }
    c = floyd_context_new(adj_matrix->columns);
    if(c == NULL) {
        show_error(window, "Unable to allocate enough memory for "
                           "this problem. Sorry.");
        return;
    }

    /* Copy adjacency matrix and names */
    matrix_copy(adj_matrix, c->table_d);
    for(int i = 0; i < adj_matrix->columns; i++) {
        c->names[i] = names[i];
    }

    /* Execute algorithm */
    bool success = floyd(c);
    if(!success) {
        show_error(window, "Error while processing the information.\n"
                           "Please check your data.");
    }

    /* Generate report */
    bool report_created = floyd_report(c);
    if(!report_created) {
        show_error(window, "Report could not be created.\n"
                           "Please check your data.");
    } else {
        printf("Report created at reports/floyd.tex\n");

        int as_pdf = latex2pdf("floyd", "reports");
        if(as_pdf == 0) {
            printf("PDF version available at reports/floyd.pdf\n");
        } else {
            char* error = g_strdup_printf("Unable to convert report to PDF.\n"
                                          "Status: %i.", as_pdf);
            show_error(window, error);
            g_free(error);
        }
    }
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
    /* Number of nodes */
    int num_nodes = adj_matrix->columns;
    fprintf(file, "%i\n", num_nodes);

    /* Nodes names */
    for(int i = 0; i < num_nodes; i++) {
        fprintf(file, "%s\n", names[i]);
    }

    /* Adjacency matrix */
    for(int i = 0; i < adj_matrix->rows; i++) {
        for(int j = 0; j < adj_matrix->columns; j++) {
            float d = adj_matrix->data[i][j];
            if(d == PLUS_INF) {
                fprintf(file, "oo ");
            } else {
                fprintf(file, "%i ", (int)d);
            }
        }
        fprintf(file, "\n");
    }
}

void load(FILE* file)
{
    /* Load number of nodes */
    int num_nodes = 0;
    fscanf(file, "%i%*c", &num_nodes);

    /* Adapt GUI */
    bool success = change_matrix(num_nodes);
    if(!success) {
        show_error(window, "Unable to allocate enough memory for "
                           "this problem. Sorry.");
        return;
    }
    gtk_spin_button_set_value(nodes, (gdouble)num_nodes);

    /* Load node names */
    GtkListStore* model = GTK_LIST_STORE(gtk_tree_view_get_model(input));
    GtkTreeIter iter;
    bool has_row = gtk_tree_model_get_iter_first(
                            GTK_TREE_MODEL(model), &iter);
    if(!has_row) {
        return;
    }

    char* name_i = NULL;
    GValue name_v = G_VALUE_INIT;

    for(int i = 0; i < num_nodes; i++) {
        /* Get name */
        name_i = get_line(file);
        /* Get create GValue */
        g_value_init(&name_v, G_TYPE_STRING);
        g_value_set_string(&name_v, name_i);
        /* Store */
        gtk_list_store_set_value(model, &iter, i + 1, &name_v);
        names[i] = g_strdup(name_i);
        /* Free resources */
        g_value_unset(&name_v);
        free(name_i);
    }

    /* Load adjacency matrix */
    gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);

    char cell_i[12];
    GValue cell_v = G_VALUE_INIT;

    for(int i = 0; i < num_nodes; i++) {
        for(int j = 0; j < num_nodes; j++) {
            /* Get string */
            fscanf(file, "%s", cell_i);

            /* Set the cell */
            if(i != j) {
                g_value_init(&cell_v, G_TYPE_STRING);

                /* Check if INFINITY */
                if(strncmp((char*) &cell_i, "oo", 2) == 0) {
                    adj_matrix->data[i][j] = PLUS_INF;
                    g_value_set_string(&cell_v, "oo");
                } else {
                    int cell = atoi((char*) &cell_i);
                    adj_matrix->data[i][j] = cell;
                    g_value_set_string(&cell_v, (char*) &cell_i);
                }

                /* Set value in GUI */
                gtk_list_store_set_value(model, &iter, j + 1, &cell_v);
                g_value_unset(&cell_v);

                /* Load name */
                g_value_init(&cell_v, G_TYPE_STRING);
                g_value_set_string(&cell_v, names[i]);
                gtk_list_store_set_value(model, &iter, 0, &cell_v);
                g_value_unset(&cell_v);
            }
        }
        gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
    }

    update_graph();
}
