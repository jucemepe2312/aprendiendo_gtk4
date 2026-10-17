#include <gtk/gtk.h>

static void print_hola(GtkWidget *widget, gpointer data) {
  g_print("hola mundo");
}

GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
gtk_box_set_homogeneous(GTK_BOX(box), TRUE);
gtk_window_set_child(GTK_WINDOW(win), box);

static void on_activate(GtkApplication *miprograma, gpointer userdata) {
  GtkWidget *ventana = gtk_application_window_new(miprograma);
  gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 320);
}

int main(int argc, char **argv) {
  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_set_homogeneous (GTK_BOX (box), TRUE);
  gtk_window_set_child (GTK_WINDOW (win), box);
}
  
