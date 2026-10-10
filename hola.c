#include <gtk/gtk.h>

static void on_activate(GtkApplication *miprograma, gpointer userdata){
  GtkWidget *ventana = gtk_application_window_new(miprograma);
  gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 320);
  gtk_window_present(GTK_WINDOW(ventana));
}

int main(int argc, char ** argv){
  GtkApplication *ventana = gtk_application_new("com.julio.github", G_APPLICATION_DEFAULT_FLAGS);
  
  g_signal_connect(ventana, "activate", G_CALLBACK(on_activate), NULL);
  
  int estado = g_application_run(G_APPLICATION(ventana), argc, argv);
  g_object_unref(ventana);
  return estado;
}

