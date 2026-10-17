#include <gtk/gtk.h>

static void print_hola(GtkWidget *widget, gpointer data) {
  g_print("hola mundo");
}

static void print_hola2(GtkWidget *widget, gpointer data) {
  g_print("hola mundo");
}



static void on_activate(GtkApplication *miprograma, gpointer userdata) {
  GtkWidget *ventana = gtk_application_window_new(miprograma);
  gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 320);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_set_homogeneous(GTK_BOX(box), TRUE);
  gtk_window_set_child(GTK_WINDOW(win), box);
 
}

int main(int argc, char **argv) {
   GtkApplication *app;
   int stat;
 
   app = gtk_application_new ("com.github.ToshioCP.lb4", G_APPLICATION_FLAGS_NONE);
   g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
   stat =g_application_run (G_APPLICATION (app), argc, argv);
   g_object_unref (app);
   return stat;
}