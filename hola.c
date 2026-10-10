#include <gtk/gtk.h>

static void print_hello (GtkWidget *widget, gpointer   data){
  g_print ("Hello World\n");
}

static void on_activate(GtkApplication *miprograma, gpointer userdata){
  GtkWidget *ventana = gtk_application_window_new(miprograma);
  gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 320);
  

  GtkWidget *button = gtk_button_new_with_label("clic aqui");
  gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button, GTK_ALIGN_CENTER);
  g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
  gtk_window_set_child(GTK_WINDOW(ventana), button);
  gtk_window_present(GTK_WINDOW(ventana));
}

int main(int argc, char ** argv){
  GtkApplication *ventana = gtk_application_new("com.julio.github", G_APPLICATION_DEFAULT_FLAGS);
  
  g_signal_connect(ventana, "activate", G_CALLBACK(on_activate), NULL);
  
  int estado = g_application_run(G_APPLICATION(ventana), argc, argv);
  g_object_unref(ventana);
  return estado;
}

