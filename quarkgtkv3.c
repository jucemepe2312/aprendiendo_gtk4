#include <gtk/gtk.h>

static void on_activate(GApplication* app, gpointer user_data)
{
    g_print("necesitas un argumento.\n");
}

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
    g_free(nombre);

    gtk_notebook_append_page(nb, scr, lab);
    g_object_set(gtk_notebook_get_page(nb, scr), "tab-expand", TRUE, NULL);
}

static void abierto(
    GApplication* app,
    GFile**files,
    gint n_files,
    gchar* hint,
    gpointer user_data)
{
    int i;
    GtkWidget* ventana;

    GtkWidget* nb;

    GtkBuilder *build;
    build = gtk_builder_new_from_file("quarkgtk.ui");
    ventana = GTK_WIDGET(gtk_builder_get_object(build, "win"));
    nb = GTK_WIDGET(gtk_builder_get_object(build, "nb"));
    gtk_window_set_application(GTK_WINDOW(ventana), GTK_APPLICATION(app));


    g_object_unref(build);

    for (i = 0; i < n_files; i++)
    {
        abrir_archivo(files[i], GTK_NOTEBOOK(nb));
    }

    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb)) > 0)
    {
        gtk_window_present(GTK_WINDOW(ventana));
    }
    else
    {
        gtk_window_destroy(GTK_WINDOW(ventana));
    }
}

int main(int argc, char**argv)
{
    GtkApplication* app;
    int stat;

    app = gtk_application_new("julio.com.visor", G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(app, "open", G_CALLBACK(abierto), NULL);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return stat;
}