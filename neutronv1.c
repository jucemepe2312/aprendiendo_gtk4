#include <gtk/gtk.h>

static void open_finish(GObject* source, GAsyncResult* result, gpointer user_data)
{
    GError* error = NULL;
    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GFile* file = gtk_file_dialog_open_finish(dialog, result, &error);
    if (file != NULL)
    {
        /*aqui hay que hacer algo*/
    }
    else if (error != NULL)
    {
        g_error_free(error);
    }
}

static void on_activate(GApplication* app, gpointer user_data)
{

    gchar* texto =
        "";

    GtkWidget* ventana = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_default_size(GTK_WINDOW(ventana), 800, 800);
    gtk_window_set_title(GTK_WINDOW(ventana), "Neutron");

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_set_homogeneous(GTK_BOX(box), TRUE);

    GtkWidget* scr = gtk_scrolled_window_new();

    GtkWidget* tv = gtk_text_view_new();
    GtkTextBuffer* tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
    gtk_text_buffer_set_text(tb, texto, -1);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_WORD_CHAR);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr), tv);
    gtk_box_append(GTK_BOX(box), scr);
    gtk_window_set_child(GTK_WINDOW(ventana), box);

    gtk_window_present(GTK_WINDOW(ventana));
}

int main(int argc, char**argv)
{
    GtkApplication* app;
    int stat;

    app = gtk_application_new("julio.github.com", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return stat;
}