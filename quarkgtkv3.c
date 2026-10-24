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
    GtkWidget* boxv;
    GtkWidget* nb;
    GtkWidget* boxh;
    GtkWidget* dmy1;
    GtkWidget* dmy2;
    GtkWidget* dmy3;
    GtkWidget* boton_nuevo;
    GtkWidget* boton_cerrar;
    GtkWidget* boton_abrir;
    GtkWidget* boton_guardar;

    ventana = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(ventana), "Visor de archivos");
    gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 320);

    boxv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(ventana), boxv);

    boxh = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append(GTK_BOX(boxv), boxh);

    dmy1 = gtk_label_new(NULL);
    gtk_label_set_width_chars(GTK_LABEL(dmy1), 10);
    dmy2 = gtk_label_new(NULL);
    gtk_widget_set_hexpand(dmy2, TRUE);
    dmy3 = gtk_label_new(NULL);
    gtk_label_set_width_chars(GTK_LABEL(dmy3), 10);

    boton_nuevo = gtk_button_new_with_label("nuevo");
    boton_cerrar = gtk_button_new_with_label("cerrar");
    boton_abrir = gtk_button_new_with_label("abrir");
    boton_guardar = gtk_button_new_with_label("guardar");

    gtk_box_append(GTK_BOX(boxh), dmy1);
    gtk_box_append(GTK_BOX(boxh), btnn);
    gtk_box_append(GTK_BOX(boxh), btno);
    gtk_box_append(GTK_BOX(boxh), dmy2);
    gtk_box_append(GTK_BOX(boxh), btns);
    gtk_box_append(GTK_BOX(boxh), btnc);
    gtk_box_append(GTK_BOX(boxh), dmy3);

    nb = gtk_notebook_new();
    gtk_box_append(GTK_BOX(boxv), nb);

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