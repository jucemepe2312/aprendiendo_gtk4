#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#define APPLICATION_ID "julio.github.com"

static GtkWidget* ventana_global = NULL;
static GtkWidget* nb_global = NULL;

static void cerrar_pestana_especifica(GtkButton* btn, gpointer user_data)
{
    GtkWidget* scr = GTK_WIDGET(user_data);
    GtkNotebook* nb = GTK_NOTEBOOK(gtk_widget_get_ancestor(scr, GTK_TYPE_NOTEBOOK));
    int idx = gtk_notebook_page_num(nb, scr);
    if (idx >= 0)
    {
        gtk_notebook_remove_page(nb, idx);
    }

    if (gtk_notebook_get_n_pages(nb) == 0)
    {
        gtk_window_destroy(GTK_WINDOW(ventana_global));
    }
}

static void abrir_archivo(GFile* archivo, GtkNotebook* nb)
{
    char* contenido = NULL;
    gsize length = 0;
    char* nombre = NULL;
    GtkWidget* lab = NULL;
    GtkWidget* scr = NULL;
    GtkWidget* sv = NULL;
    GtkSourceBuffer* sb = NULL;

    if (!g_file_load_contents(archivo, NULL, &contenido, &length, NULL, NULL))
    {
        g_print("no se pudo leer el archivo\n");
        return;
    }

    scr = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scr, TRUE);

    sv = gtk_source_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(sv), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(sv), TRUE);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(sv), TRUE);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(sv), TRUE);

    sb = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(sv)));
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(sb), contenido, length);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr), sv);
    g_free(contenido);

    nombre = g_file_get_basename(archivo);
    lab = gtk_label_new(nombre != NULL ? nombre : "");

    GtkWidget* btn = gtk_button_new_from_icon_name("window-close");
    gtk_button_set_has_frame(GTK_BUTTON(btn), FALSE);
    gtk_widget_set_focus_on_click(btn, FALSE);

    GtkWidget* box_tab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append(GTK_BOX(box_tab), lab);
    gtk_box_append(GTK_BOX(box_tab), btn);
    g_signal_connect(btn, "clicked", G_CALLBACK(cerrar_pestana_especifica), scr);
    gtk_notebook_append_page(nb, scr, box_tab);
    g_free(nombre);

    g_object_set(gtk_notebook_get_page(nb, scr), "tab-expand", TRUE, NULL);
    nombre = g_file_get_basename(archivo);
    lab = gtk_label_new(nombre != NULL ? nombre : "");

    GtkSourceLanguageManager* lm = gtk_source_language_manager_get_default();
    GtkSourceLanguage* lang = gtk_source_language_manager_guess_language(lm, nombre, NULL);
    if (lang != NULL)
    {
        gtk_source_buffer_set_language(sb, lang);
    }

    g_free(nombre); // ahora sí, después de usar nombre para adivinar
}

static gboolean save_file(GFile* file, GtkTextBuffer* tb, GtkWindow* win)
{
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    gchar* contenido;
    gboolean stat;
    GtkWidget* message_dialog;
    GError* err = NULL;

    gtk_text_buffer_get_bounds(tb, &start_iter, &end_iter);
    contenido = gtk_text_buffer_get_text(tb, &start_iter, &end_iter, FALSE);
    if (g_file_replace_contents(file, contenido, strlen(contenido), NULL, TRUE, G_FILE_CREATE_NONE,
                                NULL, NULL, &err))
    {
        gtk_text_buffer_set_modified(tb, FALSE);
        stat = TRUE;
    }
    else
    {
        message_dialog = gtk_message_dialog_new(win, GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
            "%s.\n", err->message);
        g_signal_connect(message_dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
        gtk_widget_set_visible(message_dialog, TRUE);
        g_error_free(err);
        stat = FALSE;
    }

    g_free(contenido);
    return stat;
}

static void open_finish(GObject* source, GAsyncResult* result, gpointer user_data)
{
    GError* error = NULL;
    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GFile* file = gtk_file_dialog_open_finish(dialog, result, &error);
    if (file != NULL)
    {
        abrir_archivo(file, GTK_NOTEBOOK(user_data));
        g_object_unref(file);
    }
    else if (error != NULL)
    {
        g_error_free(error);
    }
}

static void open_cb(GtkNotebook* nb)
{
    GtkFileDialog* dialog = gtk_file_dialog_new();
    GtkWindow* win = GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(nb), GTK_TYPE_WINDOW));
    gtk_file_dialog_open(dialog, win, NULL, open_finish, nb);
    g_object_unref(dialog);
}

static void save_finish(GObject* source, GAsyncResult* result, gpointer user_data)
{
    GtkTextBuffer* tb = GTK_TEXT_BUFFER(user_data);
    GError* error = NULL;
    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GFile* file = gtk_file_dialog_save_finish(dialog, result, &error);
    if (file != NULL)
    {
        save_file(file, tb, GTK_WINDOW(ventana_global));
        g_object_unref(file);
    }
    else if (error != NULL)
    {
        g_error_free(error);
    }
}

static void save_cb(GtkNotebook* nb)
{
    int idx = gtk_notebook_get_current_page(nb);
    if (idx < 0)
    {
        return;
    }

    GtkWidget* scr = gtk_notebook_get_nth_page(nb, idx);
    GtkWidget* sv = gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(scr));
    GtkTextBuffer* tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sv));
    GtkFileDialog* dialog = gtk_file_dialog_new();
    gtk_file_dialog_save(dialog, GTK_WINDOW(ventana_global), NULL, save_finish, tb);
    g_object_unref(dialog);
}

static void inicio(GApplication* app, gpointer user_data)
{
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
                                    "window { background-color: #1e1e2e; }"
        "textview { background-color: #010112; color: #cdd6f4; padding: 12px; font-family: 'JetBrains Mono', 'Fira Code', monospace; font-size: 12pt; caret-color: #cba6f7; }"
        "textview text { background-color: #1e1e2e; color: #cdd6f4; }"
        "notebook { background-color: #010115; }"
        "notebook > header { background-color: #01010f; border: none; }"
        "notebook > header > tabs > tab { background-color: #181825; color: #a6adc8; padding: 6px 12px; border-radius: 6px 6px 0 0; margin: 2px; }"
        "notebook > header > tabs > tab:checked { background-color: #010216; color: #cdd6f4; }"
        "notebook > header > tabs > tab button { background: transparent; border: none; padding: 0; min-width: 16px; min-height: 16px; color: #a6adc8; }"
        "notebook > header > tabs > tab button:hover { background-color: #f38ba8; border-radius: 50%; color: #1e1e2e; }"
        "button { background-color: #313244; color: #cdd6f4; border: none; border-radius: 6px; padding: 8px 14px; margin: 4px; font-weight: bold; }"
        "button:hover { background-color: #45475a; }"
        "button:active { background-color: #cba6f7; color: #1e1e2e; }"
        "scrolledwindow { background-color: #00000a; }"
        "scrollbar { background-color: #181825; }"
        "scrollbar slider { background-color: #45475a; border-radius: 6px; min-width: 8px; min-height: 8px; }"
        "scrollbar slider:hover { background-color: #cba6f7; }",
        -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    GtkBuilder* build;
    GtkButton* btno;
    GtkButton* btns;

    build = gtk_builder_new_from_file("quarkgtk.ui");
    ventana_global = GTK_WIDGET(gtk_builder_get_object(build, "win"));
    nb_global = GTK_WIDGET(gtk_builder_get_object(build, "nb"));
    gtk_window_set_application(GTK_WINDOW(ventana_global), GTK_APPLICATION(app));

    btno = GTK_BUTTON(gtk_builder_get_object(build, "btno"));
    btns = GTK_BUTTON(gtk_builder_get_object(build, "btns"));

    if (btno)
    {
        g_signal_connect_swapped(btno, "clicked", G_CALLBACK(open_cb), nb_global);
    }

    if (btns)
    {
        g_signal_connect_swapped(btns, "clicked", G_CALLBACK(save_cb), nb_global);
    }

    g_object_unref(build);
}

static void on_activate(GApplication* application)
{
    gtk_window_present(GTK_WINDOW(ventana_global));
}

static void abierto(GApplication* application, GFile**files, gint n_files, const gchar* hint)
{
    int i;
    for (i = 0; i < n_files; i++)
    {
        abrir_archivo(files[i], GTK_NOTEBOOK(nb_global));
    }

    gtk_window_present(GTK_WINDOW(ventana_global));
}

int main(int argc, char**argv)
{
    GtkApplication* app;
    int stat;
    app = gtk_application_new(APPLICATION_ID, G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "startup", G_CALLBACK(inicio), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(app, "open", G_CALLBACK(abierto), NULL);
    stat = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return stat;
}