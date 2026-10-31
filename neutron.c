#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/stat.h>

void listar_archivos(char archivos[][256], int* num_archivos)
{
    DIR* dir = opendir("/sys/block");
    if (dir == NULL)
    {
        *num_archivos = 0;
        return;
    }

    struct dirent* entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL && count < MAX_ARCHIVOS)
    {
        if (strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0)
        {
            strcpy(archivos[count], entry->d_name);
            count++;
            printf("%s\n", entry->d_name);
        }
    }

    *num_archivos = count;
    closedir(dir);
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