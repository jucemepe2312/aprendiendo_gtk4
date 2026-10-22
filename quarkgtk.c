#include <gtk/gtk.h>


static void on_activate(GApplication *app, gpointer user_data) {
    g_print ("You need a filename argument.\n");
 }

static void abierto(GApplication *app, GFile ** files, gint n_files, gchar *hint, gpointer user_data) {

    char *contenido;
    gsize lenght;
    char *nombre_de_archivo;

  GtkWidget *ventana = gtk_application_window_new(GTK_APPLICATION(app));
  gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 320);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_set_homogeneous(GTK_BOX(box), TRUE);
  gtk_window_set_child(GTK_WINDOW(ventana), box);

  GtkWidget *scr = gtk_scrolled_window_new();


  GtkWidget *tv = gtk_text_view_new();

  GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tv), GTK_WRAP_WORD_CHAR);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tv), FALSE);

  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scr), tv);

  if (g_file_load_contents(files[0], NULL, &contenido, &lenght, NULL, NULL)){
    gtk_text_buffer_set_text (tb, contenido, lenght);
    g_free(contenido);

    if ((nombre_de_archivo = g_file_get_basename(files[0])) != NULL){
        gtk_window_set_title (GTK_WINDOW (ventana), nombre_de_archivo);
        g_free (nombre_de_archivo);
    }
    gtk_window_present(GTK_WINDOW(ventana));
  } else {
    if ((nombre_de_archivo = g_file_get_path(files[0])) != NULL){
        g_print ("No such file: %s.\n", nombre_de_archivo);
        g_free (nombre_de_archivo);
    }
    gtk_window_destroy(GTK_WINDOW(ventana));
  }

  gtk_box_append(GTK_BOX(box), scr);
  gtk_window_set_child(GTK_WINDOW(ventana), box);
  
 
}

int main(int argc, char **argv) {
   GtkApplication *app;
   int stat;
 
   app = gtk_application_new ("julio.com.com", G_APPLICATION_HANDLES_OPEN);
   g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
   g_signal_connect (app, "open", G_CALLBACK (abierto), NULL);
   stat =g_application_run (G_APPLICATION (app), argc, argv);
   g_object_unref (app);
   return stat;
}