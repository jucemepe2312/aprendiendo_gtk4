#include <gtk/gtk.h>
static GtkWidget* ventana_global = NULL;
static GtkWidget* nb_global = NULL;
static void cerrar_pestana_especifica(GtkButton *btn, gpointer user_data)
{
    GtkWidget *scr = GTK_WIDGET(user_data);
    GtkNotebook *nb = GTK_NOTEBOOK(gtk_widget_get_ancestor(scr, GTK_TYPE_NOTEBOOK));
    int idx = gtk_notebook_page_num(nb, scr);
    if (idx >= 0) {
        gtk_notebook_remove_page(nb, idx);
    }
    if (gtk_notebook_get_n_pages(nb) == 0) {
        gtk_window_destroy(GTK_WINDOW(ventana_global));
    }
}
#define APPLICATION_ID "julio.github.com"
static void abrir_archivo(GFile* archivo, GtkNotebook* nb)
{
    char* contenido = NULL;
    gsize length = 0;
    char* nombre = NULL;
    GtkWidget* lab = NULL;
    GtkWidget* scr = NULL;
    GtkWidget* tv = NULL;
    GtkTextBuffer* tb = NULL;

    if (!g_file_load_contents(archivo, NULL, &contenido, &length, NULL, NULL))
    {
        g_print("no se pudo leer el archivo\n");
        return;
    }

    scr = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scr, TRUE);
    tv = gtk_text_view_new();
    tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), TRUE);
    gtk_text_buffer_set_text(tb, contenido, length);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr), tv);
    g_free(contenido);

    nombre = g_file_get_basename(archivo);
    lab = gtk_label_new(nombre != NULL ? nombre : "");

    GtkWidget *btn = gtk_button_new_from_icon_name("window-close");
    gtk_button_set_has_frame(GTK_BUTTON(btn), FALSE);
    gtk_widget_set_focus_on_click(btn, FALSE);
    
    GtkWidget *box_tab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append(GTK_BOX(box_tab), lab);
    gtk_box_append(GTK_BOX(box_tab), btn);
    g_signal_connect(btn, "clicked", G_CALLBACK(cerrar_pestana_especifica), scr);
    gtk_notebook_append_page(nb, scr, box_tab);
    g_free(nombre);

    
    g_object_set(gtk_notebook_get_page(nb, scr), "tab-expand", TRUE, NULL);
}


static gboolean
save_file(GFile* file, GtkTextBuffer* tb, GtkWindow* win)
{
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    gchar* contenido = gtk_text_buffer_get_text(tb, &start_iter, &end_iter, FALSE);
    gboolean stat;
    GtkWidget* message_dialog;
    GError* err = NULL;

    gtk_text_buffer_get_bounds(tb, &start_iter, &end_iter);

    contenido = gtk_text_buffer_get_text(tb, &start_iter, &end_iter, FALSE);
    if (g_file_replace_contents(file, contenido, strlen(contenido), NULL, TRUE, G_FILE_CREATE_NONE,
                                NULL,
        NULL, &err))
    {
        gtk_text_buffer_set_modified(tb, FALSE);
        stat = TRUE;
    }
    else
    {
        message_dialog = gtk_message_dialog_new(win, GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
            "%s.\n", err->message);
        g_signal_connect(message_dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
        gtk_widget_set_visible(message_dialog, TRUE);
        g_error_free(err);
        stat = FALSE;
    }

    g_free(contenido);
    return stat;
}



static void
open_finish(GObject* source, GAsyncResult* result, gpointer user_data)
{
    GError* error = NULL;
    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GFile* file = gtk_file_dialog_open_finish(dialog, result, &error);
    if (file != NULL)
    {
        abrir_archivo(file, GTK_NOTEBOOK(user_data));
        g_object_unref(file);
    }
}

void
cerrar_pestaña(GtkNotebook* nb)
{
    g_return_if_fail(GTK_IS_NOTEBOOK(nb));

    GtkWidget* win;
    int i;

    if (gtk_notebook_get_n_pages(nb) == 1)
    {
        win = gtk_widget_get_ancestor(GTK_WIDGET(nb), GTK_TYPE_WINDOW);
        gtk_window_destroy(GTK_WINDOW(win));
    }
    else
    {
        i = gtk_notebook_get_current_page(nb);
        gtk_notebook_remove_page(GTK_NOTEBOOK(nb), i);
    }
}

static void open_cb(GtkNotebook* nb)
{
    g_print("open_cb ejecutado, nb=%p\n", nb);
    GtkFileDialog* dialog = gtk_file_dialog_new();
    GtkWindow* win = GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(nb), GTK_TYPE_WINDOW));
    gtk_file_dialog_open(dialog, win, NULL, open_finish, nb);
    g_object_unref(dialog);
}

/*static void
new_cb(GtkNotebook* nb)
{
    notebook_page_new(nb);
}

static void
save_cb(GtkNotebook* nb)
{
    notebook_page_save(nb);
}

static void
close_cb(GtkNotebook* nb)
{
    notebook_page_close(GTK_NOTEBOOK(nb));
}*/

static void inicio(GApplication* app, gpointer user_data)
{
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
                                      "textview { padding: 10px; font-family: monospace; font-size: 12pt; }");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    int i;

    GtkBuilder* build;

    GtkButton* btno;
    GtkButton* btnn;
    GtkButton* btns;
    GtkButton* btnc;

    build = gtk_builder_new_from_file("quarkgtk.ui");
    ventana_global = GTK_WIDGET(gtk_builder_get_object(build, "win"));
    nb_global = GTK_WIDGET(gtk_builder_get_object(build, "nb"));

    gtk_window_set_application(GTK_WINDOW(ventana_global), GTK_APPLICATION(app));

    btno = GTK_BUTTON(gtk_builder_get_object(build, "btno"));
    btnn = GTK_BUTTON(gtk_builder_get_object(build, "btnn"));
    btns = GTK_BUTTON(gtk_builder_get_object(build, "btns"));
    btnc = GTK_BUTTON(gtk_builder_get_object(build, "btnc"));
    g_print("btno=%p, nb_global=%p\n", btno, nb_global);
    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb_global)) == 0)
    {
        gtk_window_present(GTK_WINDOW(ventana_global));
    }

    g_signal_connect_swapped(btno, "clicked", G_CALLBACK(open_cb), nb_global);
    /*g_signal_connect_swapped(btnn, "clicked", G_CALLBACK(new_cb), nb);
    g_signal_connect_swapped(btns, "clicked", G_CALLBACK(save_cb), nb);*/
    g_signal_connect_swapped(btnc, "clicked", G_CALLBACK(cerrar_pestaña), nb_global);
    g_object_unref(build);
}

static void
on_activate(GApplication* application)
{

    gtk_window_present(GTK_WINDOW(ventana_global));
}

static void
abierto(GApplication* application, GFile**files, gint n_files, const gchar* hint)
{

    int i;

    for (i = 0; i < n_files; i++)
    {
        abrir_archivo(files[i], GTK_NOTEBOOK(nb_global));
    }

    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb_global)) == 0)
    {
        gtk_window_present(GTK_WINDOW(ventana_global));
    }

}

int main(int argc, char**argv)
{
    GtkApplication* app;
    int stat;

    app = gtk_application_new(APPLICATION_ID, G_APPLICATION_HANDLES_OPEN);

    g_signal_connect(app, "startup", G_CALLBACK(inicio), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(app, "open", G_CALLBACK(abierto), NULL);

    stat = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return stat;
}