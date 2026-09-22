#include <gtk/gtk.h>
static void on_activate(GtkApplication *app, gpointer userdata){
GtkWidget *window = gtk_application_window_new(app);
gtk_window_set_title(GTK_WINDOW(window), "hola desde gtk4");

GtkWidget  *button = gtk_button_new_with_label("haz clic");
GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
gtk_box_append(GTK_BOX(box), button);
gtk_window_set_child(GTK_WINDOW(window), box);

gtk_window_present(GTK_WINDOW(window));

}

int main(int argc, char **argv){
      GtkApplication *app;
     int stat;
    app = gtk_application_new ("com.github.ToshioCP.pr1", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
 stat =g_application_run (G_APPLICATION (app), argc,  argv);
   g_object_unref (app);
   return stat;
}
