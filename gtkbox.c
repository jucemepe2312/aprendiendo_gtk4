#include <gtk/gtk.h>

/*finalizado, he hecho un gtkbox que puede tener muchos hijos*/

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
  gtk_window_set_child(GTK_WINDOW(ventana), box);

  GtkWidget *boton = gtk_button_new_with_label("un boton en box");
  gtk_box_append(GTK_BOX(box), boton);
  gtk_widget_set_halign(boton, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(boton, GTK_ALIGN_CENTER);
  g_signal_connect(boton, "clicked", G_CALLBACK(print_hola), NULL);

  gtk_window_set_child(GTK_WINDOW(ventana), box);




  gtk_window_present(GTK_WINDOW(ventana));
 
}

int main(int argc, char **argv) {
   GtkApplication *app;
   int stat;
 
   app = gtk_application_new ("julio.com.com", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
   stat =g_application_run (G_APPLICATION (app), argc, argv);
   g_object_unref (app);
   return stat;
}