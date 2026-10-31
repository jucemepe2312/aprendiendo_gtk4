#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_CHARS 256
#define MAX_ARCHIVOS 100

int grabar_iso(const char* ruta_iso, const char* dispositivo)
{
    char buffer[4096];
    ssize_t n;
    int fd_iso = open(ruta_iso, O_RDONLY);
    if (fd_iso == -1)
    {
        perror("open iso");
        return -1;
    }

    int fd_usb = open(dispositivo, O_WRONLY);
    if (fd_usb == -1)
    {
        perror("open usb");
        close(fd_iso);
        return -1;
    }

    while ((n = read(fd_iso, buffer, sizeof(buffer))) > 0)
    {
        if (write(fd_usb, buffer, n) != n)
        {
            perror("write");
            close(fd_iso);
            close(fd_usb);
            return -1;
        }
    }

    if (n == -1)
    {
        perror("read");
    }

    fsync(fd_usb);
    close(fd_iso);
    close(fd_usb);
    return 0;
}

void leer_modelo(const char* nombre, char* modelo, size_t tam)
{
    char ruta[256];
    snprintf(ruta, sizeof(ruta), "/sys/block/%s/device/model", nombre);
    FILE* f = fopen(ruta, "r");
    if (f == NULL)
    {
        snprintf(modelo, tam, "Desconocido");
        return;
    }

    if (fgets(modelo, tam, f) == NULL)
    {
        snprintf(modelo, tam, "Desconocido");
    }
    else
    {
        modelo[strcspn(modelo, "\n")] = '\0';
    }

    fclose(f);
}

double leer_tamano_gb(const char* nombre)
{
    char ruta[256];
    snprintf(ruta, sizeof(ruta), "/sys/block/%s/size", nombre);
    FILE* f = fopen(ruta, "r");
    if (f == NULL)
    {
        return 0.0;
    }

    unsigned long long sectores = 0;
    if (fscanf(f, "%llu", &sectores) != 1)
    {
        fclose(f);
        return 0.0;
    }

    fclose(f);
    return (sectores * 512.0) /(1024.0 * 1024.0 * 1024.0);
}

void listar_archivos(
    char archivos[][MAX_CHARS],
    char modelos[][MAX_CHARS],
    double tamanos[],
    int* num_archivos)
{
    char ruta[256];
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
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(ruta, sizeof(ruta), "/sys/block/%s/removable", entry->d_name);
        FILE* f = fopen(ruta, "r");
        if (f == NULL)
        {
            continue;
        }

        int c = fgetc(f);
        fclose(f);

        if (c == '1')
        {
            strncpy(archivos[count], entry->d_name, MAX_CHARS - 1);
            archivos[count][MAX_CHARS - 1] = '\0';
            leer_modelo(entry->d_name, modelos[count], MAX_CHARS);
            tamanos[count] = leer_tamano_gb(entry->d_name);
            count++;
        }
    }

    *num_archivos = count;
    closedir(dir);
}

int main(void)
{
    char archivos[MAX_ARCHIVOS][MAX_CHARS];
    char modelos[MAX_ARCHIVOS][MAX_CHARS];
    double tamanos[MAX_ARCHIVOS];
    int num_archivos = 0;
    char ruta_iso[256];
    char dispositivo[256];

    printf("BIENVENIDO A NEUTRON TUI\n\n");
    listar_archivos(archivos, modelos, tamanos, &num_archivos);

    if (num_archivos == 0)
    {
        printf("No se encontraron dispositivos extraíbles.\n");
        return 1;
    }

    printf("Dispositivos disponibles:\n");
    for (int i = 0; i < num_archivos; i++)
    {
        printf("  %d) /dev/%s — %s (%.1f GB)\n",
               i + 1, archivos[i], modelos[i], tamanos[i]);
    }

    int eleccion;
    printf("\nElige el número del dispositivo: ");
    if (scanf("%d", &eleccion) != 1 || eleccion < 1 || eleccion > num_archivos)
    {
        printf("Elección inválida.\n");
        return 1;
    }

    int idx = eleccion - 1;
    snprintf(dispositivo, sizeof(dispositivo), "/dev/%s", archivos[idx]);

    printf("\nRuta de la ISO: ");
    if (scanf("%255s", ruta_iso) != 1)
    {
        printf("Ruta inválida.\n");
        return 1;
    }

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  CONFIRMACIÓN                                ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║  Dispositivo: /dev/%-24s  ║\n", archivos[idx]);
    printf("║  Modelo:      %-30s ║\n", modelos[idx]);
    printf("║  Tamaño:    %-29.1f GB ║\n", tamanos[idx]);
    printf("║  ISO:         %-30s ║\n", ruta_iso);
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf("Se BORRARÁN todos los datos de /dev/%s.\n", archivos[idx]);
    printf("Escribe '%s' para confirmar (o 'n' para cancelar): ", archivos[idx]);

    char confirmacion[MAX_CHARS];
    if (scanf("%255s", confirmacion) != 1)
    {
        printf("Cancelado.\n");
        return 1;
    }

    if (strcmp(confirmacion, archivos[idx]) != 0)
    {
        printf("Confirmación incorrecta. Cancelado.\n");
        return 0;
    }

    printf("\nGrabando %s en %s...\n", ruta_iso, dispositivo);
    if (grabar_iso(ruta_iso, dispositivo) == 0)
    {
        printf(" Grabación completada.\n");
    }
    else
    {
        printf(" Error al grabar.\n");
        return 1;
    }

    return 0;
}