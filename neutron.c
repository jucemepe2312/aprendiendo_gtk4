#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define MAX_ARCHIVOS 100
#define MAX_CHARS 256

/* ---------- Estado global ---------- */
static GtkWidget *ventana_global        = NULL;
static GtkWidget *dropdown_dispositivos = NULL;
static GtkWidget *label_iso             = NULL;
static GtkWidget *barra_progreso        = NULL;
static GtkWidget *label_estado          = NULL;
static GtkWidget *boton_grabar          = NULL;
static GtkWidget *boton_seleccionar     = NULL;

static char *ruta_iso_seleccionada = NULL;
static char *dispositivo_seleccionado = NULL;
static GtkStringList *lista_dispositivos = NULL;

typedef struct {
    char *ruta_iso;
    char *dispositivo;
} DatosGrabacion;

/* ---------- Helpers de UI thread-safe ---------- */

static void ui_set_label(gpointer data)
{
    gtk_label_set_text(GTK_LABEL(label_estado), (const char *)data);
    g_free(data);
}

static void ui_status(const char *txt)
{
    g_idle_add_once(ui_set_label, g_strdup(txt));
}

typedef struct { double frac; } FracData;
static void ui_set_fraction(gpointer data)
{
    FracData *f = data;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(barra_progreso), f->frac);
    g_free(f);
}
static void ui_progress(double frac)
{
    FracData *f = g_new(FracData, 1);
    f->frac = frac;
    g_idle_add_once(ui_set_fraction, f);
}

typedef struct { gboolean enable; } EnableData;
static void ui_set_sensitive(gpointer data)
{
    EnableData *e = data;
    gtk_widget_set_sensitive(boton_grabar,      e->enable);
    gtk_widget_set_sensitive(boton_seleccionar, e->enable);
    g_free(e);

}
static void ui_enable_buttons(gboolean enable)
{
    EnableData *e = g_new(EnableData, 1);
    e->enable = enable;
    g_idle_add_once(ui_set_sensitive, e);
}

/* ---------- Lógica de negocio (sysfs) ---------- */

static void leer_modelo(const char *nombre, char *modelo, size_t tam)
{
    char ruta[256];
    snprintf(ruta, sizeof(ruta), "/sys/block/%s/device/model", nombre);
    FILE *f = fopen(ruta, "r");
    if (!f) { snprintf(modelo, tam, "Desconocido"); return; }
    if (!fgets(modelo, tam, f)) snprintf(modelo, tam, "Desconocido");
    else modelo[strcspn(modelo, "\n")] = '\0';
    fclose(f);
}

static double leer_tamano_gb(const char *nombre)
{
    char ruta[256];
    snprintf(ruta, sizeof(ruta), "/sys/block/%s/size", nombre);
    FILE *f = fopen(ruta, "r");
    if (!f) return 0.0;
    unsigned long long sectores = 0;
    if (fscanf(f, "%llu", &sectores) != 1) { fclose(f); return 0.0; }
    fclose(f);
    return (sectores * 512.0) / (1024.0 * 1024.0 * 1024.0);
}

static void listar_archivos(char archivos[][MAX_CHARS],
                            char modelos[][MAX_CHARS],
                            double tamanos[],
                            int *num_archivos)
{
    DIR *dir = opendir("/sys/block");
    if (!dir) { *num_archivos = 0; return; }
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL && count < MAX_ARCHIVOS) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char ruta[256];
        snprintf(ruta, sizeof(ruta), "/sys/block/%s/removable", entry->d_name);
        FILE *f = fopen(ruta, "r");
        if (!f) continue;
        int c = fgetc(f);
        fclose(f);
        if (c != '1') continue;

        strncpy(archivos[count], entry->d_name, MAX_CHARS - 1);
        archivos[count][MAX_CHARS - 1] = '\0';
        leer_modelo(entry->d_name, modelos[count], MAX_CHARS);
        tamanos[count] = leer_tamano_gb(entry->d_name);
        count++;
    }
    *num_archivos = count;
    closedir(dir);
}

/* ---------- Hilo de grabación ---------- */

static gpointer hilo_grabar(gpointer data)
{
    DatosGrabacion *d = data;
    char buffer[4096];
    ssize_t n;

    ui_status("Abriendo ISO...");
    int fd_iso = open(d->ruta_iso, O_RDONLY);
    if (fd_iso == -1) {
        ui_status("Error: no se pudo abrir la ISO.");
        goto cleanup;
    }

    ui_status("Abriendo dispositivo USB...");
    int fd_usb = open(d->dispositivo, O_WRONLY | O_SYNC);
    if (fd_usb == -1) {
        ui_status("Error: no se pudo abrir el USB (¿permisos? ¿está montado?).");
        close(fd_iso);
        goto cleanup;
    }

    off_t total = lseek(fd_iso, 0, SEEK_END);
    lseek(fd_iso, 0, SEEK_SET);
    off_t escrito = 0;

    ui_status("Grabando...");
    gboolean ok = TRUE;

    while ((n = read(fd_iso, buffer, sizeof(buffer))) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = write(fd_usb, buffer + off, n - off);
            if (w < 0) {
                if (errno == EINTR) continue;
                ok = FALSE;
                break;
            }
            off += w;
        }
        if (!ok) break;
        escrito += n;
        if (total > 0) ui_progress((double)escrito / (double)total);
    }
    if (n < 0) ok = FALSE;

    /* Forzar escritura física y avisar al kernel */
    if (fd_usb >= 0) {
        fsync(fd_usb);
#ifdef BLKFLSBUF
        ioctl(fd_usb, BLKFLSBUF, 0);
#endif
    }
    if (fd_iso >= 0) close(fd_iso);
    if (fd_usb >= 0) close(fd_usb);

    if (ok) {
        ui_progress(1.0);
        ui_status("✅ ¡Grabación completada con éxito!");
    } else {
        ui_status("❌ Error durante la grabación.");
    }

cleanup:
    g_free(d->ruta_iso);
    g_free(d->dispositivo);
    g_free(d);
    ui_enable_buttons(TRUE);
    return NULL;
}

/* ---------- Diálogo de confirmación (GtkMessageDialog, GTK4 base) ---------- */

static void lanzar_grabacion(void)
{
    ui_enable_buttons(FALSE);

    DatosGrabacion *d = g_new0(DatosGrabacion, 1);
    d->ruta_iso    = g_strdup(ruta_iso_seleccionada);
    d->dispositivo = g_strdup(dispositivo_seleccionado);
    g_thread_new("grabador", hilo_grabar, d);
}

static void on_confirm_response(GtkDialog *dialog, int response, gpointer user_data)
{
    if (response == GTK_RESPONSE_YES)
        lanzar_grabacion();
    gtk_window_destroy(GTK_WINDOW(dialog));
}

/* ---------- Callback del file chooser nativo ---------- */

static void on_file_chosen(GtkNativeDialog *native, int response, gpointer user_data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
        GFile *file = gtk_file_chooser_get_file(chooser);
        if (file) {
            g_free(ruta_iso_seleccionada);
            ruta_iso_seleccionada = g_file_get_path(file);
            gtk_label_set_text(GTK_LABEL(label_iso), ruta_iso_seleccionada);
            g_object_unref(file);
        }
    }
    g_object_unref(native);
}

static void on_seleccionar_iso(GtkButton *btn, gpointer user_data)
{
    GtkFileChooserNative *native = gtk_file_chooser_native_new(
        "Seleccionar ISO",
        GTK_WINDOW(ventana_global),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Abrir", "_Cancelar");

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Imágenes ISO");
    gtk_file_filter_add_pattern(filter, "*.iso");
    gtk_file_filter_add_pattern(filter, "*.img");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

    g_signal_connect(native, "response", G_CALLBACK(on_file_chosen), NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

/* ---------- Botón GRABAR ---------- */

static void on_grabar(GtkButton *btn, gpointer user_data)
{
    if (!ruta_iso_seleccionada) {
        gtk_label_set_text(GTK_LABEL(label_estado), "Error: selecciona una ISO primero.");
        return;
    }

    guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(dropdown_dispositivos));
    if (sel == GTK_INVALID_LIST_POSITION) {
        gtk_label_set_text(GTK_LABEL(label_estado), "Error: selecciona un dispositivo USB.");
        return;
    }

    GListModel *model = gtk_drop_down_get_model(GTK_DROP_DOWN(dropdown_dispositivos));
    GObject *item = g_list_model_get_item(model, sel);
    const char *txt = gtk_string_object_get_string(GTK_STRING_OBJECT(item));

    /* Formato esperado: "/dev/sdX — MODELO (Y.Y GB)" */
    char disp[64] = {0};
    if (sscanf(txt, "/dev/%63[^ —]", disp) != 1) {
        gtk_label_set_text(GTK_LABEL(label_estado), "Error: dispositivo no válido.");
        g_object_unref(item);
        return;
    }
    g_object_unref(item);

    g_free(dispositivo_seleccionado);
    dispositivo_seleccionado = g_strdup_printf("/dev/%s", disp);

    /* Confirmación real */
    GtkWidget *dlg = gtk_message_dialog_new(
        GTK_WINDOW(ventana_global),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        "¿Seguro que quieres grabar en %s?",
        dispositivo_seleccionado);

    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dlg),
        "Se borrarán TODOS los datos del dispositivo.\n\nISO: %s",
        ruta_iso_seleccionada);

    gtk_dialog_add_buttons(GTK_DIALOG(dlg),
        "_Cancelar", GTK_RESPONSE_CANCEL,
        "Sí, _grabar", GTK_RESPONSE_YES,
        NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dlg), GTK_RESPONSE_CANCEL);

    g_signal_connect(dlg, "response", G_CALLBACK(on_confirm_response), NULL);
    gtk_widget_show(dlg);
}

/* ---------- Construcción de UI ---------- */

static void on_activate(GApplication *app, gpointer user_data)
{
    ventana_global = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(ventana_global), "Neutron GTK");
    gtk_window_set_default_size(GTK_WINDOW(ventana_global), 520, 340);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_window_set_child(GTK_WINDOW(ventana_global), box);

    /* Lista de dispositivos */
    char archivos[MAX_ARCHIVOS][MAX_CHARS];
    char modelos[MAX_ARCHIVOS][MAX_CHARS];
    double tamanos[MAX_ARCHIVOS];
    int n = 0;
    listar_archivos(archivos, modelos, tamanos, &n);

    lista_dispositivos = gtk_string_list_new(NULL);
    for (int i = 0; i < n; i++) {
        char texto[512];
        snprintf(texto, sizeof(texto), "/dev/%s — %s (%.1f GB)",
                 archivos[i], modelos[i], tamanos[i]);
        gtk_string_list_append(lista_dispositivos, texto);
    }

    dropdown_dispositivos = gtk_drop_down_new(G_LIST_MODEL(lista_dispositivos), NULL);
    gtk_box_append(GTK_BOX(box), gtk_label_new("Dispositivo USB:"));
    gtk_box_append(GTK_BOX(box), dropdown_dispositivos);

    /* Selección de ISO */
    boton_seleccionar = gtk_button_new_with_label("Seleccionar ISO");
    g_signal_connect(boton_seleccionar, "clicked", G_CALLBACK(on_seleccionar_iso), NULL);
    label_iso = gtk_label_new("Ninguna ISO seleccionada");
    gtk_label_set_ellipsize(GTK_LABEL(label_iso), PANGO_ELLIPSIZE_MIDDLE);
    gtk_box_append(GTK_BOX(box), boton_seleccionar);
    gtk_box_append(GTK_BOX(box), label_iso);

    /* Progreso */
    barra_progreso = gtk_progress_bar_new();
    gtk_box_append(GTK_BOX(box), barra_progreso);

    label_estado = gtk_label_new("Listo para grabar.");
    gtk_box_append(GTK_BOX(box), label_estado);

    /* Botón grabar */
    boton_grabar = gtk_button_new_with_label("¡GRABAR!");
    gtk_widget_add_css_class(boton_grabar, "destructive-action");
    g_signal_connect(boton_grabar, "clicked", G_CALLBACK(on_grabar), NULL);
    gtk_box_append(GTK_BOX(box), boton_grabar);

    gtk_window_present(GTK_WINDOW(ventana_global));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.julio.neutron",
                                              G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}