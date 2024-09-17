#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/utsname.h>

// Definiciones para la longitud de comandos y el máximo de trozos
#define MAX_INPUT 1024
#define MAX_TROZOS 128
#define MAXNAME 256

// Estructura para la lista de comandos
typedef struct Nodo {
    char *comando;
    struct Nodo *siguiente;
} Nodo;

typedef struct Lista {
    Nodo *cabeza;
} Lista;

// Estructura para la lista de ficheros abiertos
typedef struct Fichero {
    int descriptor;
    char *nombre;
    int modo;
    struct Fichero *siguiente;
} Fichero;

typedef struct ListaFicheros {
    Fichero *cabeza;
} ListaFicheros;

// Declaraciones de funciones
void imprimirPrompt();
void leerEntrada(char *entrada);
void procesarEntrada(char *entrada, Lista *listaComandos, ListaFicheros *listaFicheros);
int TrocearCadena(char *cadena, char *trozos[]);
void AnadirComando(Lista *lista, char *comando);
void Cmd_open(char *tr[], ListaFicheros *listaFicheros);
void Cmd_close(char *tr[], ListaFicheros *listaFicheros);
void Cmd_historic(Lista *lista);
void Cmd_pid();
void Cmd_ppid();
void Cmd_date(char *tr[]);
void Cmd_infosys();
void AnadirAFicherosAbiertos(ListaFicheros *lista, int descriptor, char *nombre, int modo);
void EliminarDeFicherosAbiertos(ListaFicheros *lista, int descriptor);
void ListarFicherosAbiertos(ListaFicheros *lista);
void LiberarListaComandos(Lista *lista);
void LiberarListaFicheros(ListaFicheros *lista);

// Función principal
int main() {
    char entrada[MAX_INPUT];
    Lista listaComandos = {NULL};
    ListaFicheros listaFicheros = {NULL};

    while (1) {
        imprimirPrompt();
        leerEntrada(entrada);
        if (strlen(entrada) > 0) {
            AnadirComando(&listaComandos, entrada);
            procesarEntrada(entrada, &listaComandos, &listaFicheros);
        }
    }

    // Liberar la memoria antes de salir
    LiberarListaComandos(&listaComandos);
    LiberarListaFicheros(&listaFicheros);

    return 0;
}

// Función para imprimir el prompt
void imprimirPrompt() {
    printf("mi_shell> ");
    fflush(stdout);
}

// Función para leer la entrada del usuario
void leerEntrada(char *entrada) {
    if (fgets(entrada, MAX_INPUT, stdin) == NULL) {
        perror("Error al leer entrada");
        exit(EXIT_FAILURE);
    }
}

// Función para procesar la entrada del usuario
void procesarEntrada(char *entrada, Lista *listaComandos, ListaFicheros *listaFicheros) {
    char *trozos[MAX_TROZOS];
    int numTrozos = TrocearCadena(entrada, trozos);

    if (numTrozos == 0) return;

    if (strcmp(trozos[0], "open") == 0) {
        Cmd_open(trozos + 1, listaFicheros);
    } else if (strcmp(trozos[0], "close") == 0) {
        Cmd_close(trozos + 1, listaFicheros);
    } else if (strcmp(trozos[0], "historic") == 0) {
        Cmd_historic(listaComandos);
    } else if (strcmp(trozos[0], "pid") == 0) {
        Cmd_pid();
    } else if (strcmp(trozos[0], "ppid") == 0) {
        Cmd_ppid();
    } else if (strcmp(trozos[0], "date") == 0) {
        Cmd_date(trozos + 1);
    } else if (strcmp(trozos[0], "infosys") == 0) {
        Cmd_infosys();
    } else if (strcmp(trozos[0], "quit") == 0 || strcmp(trozos[0], "exit") == 0 || strcmp(trozos[0], "bye") == 0) {
        LiberarListaComandos(listaComandos);
        LiberarListaFicheros(listaFicheros);
        exit(0);
    } else {
        printf("Comando no reconocido: %s\n", trozos[0]);
    }
}

// Función para dividir la cadena en trozos
int TrocearCadena(char *cadena, char *trozos[]) {
    int i = 0;
    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL)
        return 0;

    while ((trozos[++i] = strtok(NULL, " \n\t")) != NULL);
    return i;
}

// Función para añadir un comando al histórico
void AnadirComando(Lista *lista, char *comando) {
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    nuevo->comando = strdup(comando);
    nuevo->siguiente = lista->cabeza;
    lista->cabeza = nuevo;
}

// Función para listar el histórico de comandos
void Cmd_historic(Lista *lista) {
    Nodo *actual = lista->cabeza;
    int contador = 0;

    while (actual != NULL) {
        printf("%d: %s\n", contador++, actual->comando);
        actual = actual->siguiente;
    }
}

// Función para obtener el PID del proceso
void Cmd_pid() {
    printf("PID: %d\n", getpid());
}

// Función para obtener el PPID del proceso
void Cmd_ppid() {
    printf("PPID: %d\n", getppid());
}

// Función para obtener la fecha y hora actual
void Cmd_date(char *tr[]) {
    time_t t;
    struct tm *tm_info;
    char buffer[80];

    time(&t);
    tm_info = localtime(&t);

    if (tr[0] == NULL) {
        strftime(buffer, 80, "%d/%m/%Y %H:%M:%S", tm_info);
        printf("Fecha y hora actual: %s\n", buffer);
    } else if (strcmp(tr[0], "-d") == 0) {
        strftime(buffer, 80, "%d/%m/%Y", tm_info);
        printf("Fecha actual: %s\n", buffer);
    } else if (strcmp(tr[0], "-t") == 0) {
        strftime(buffer, 80, "%H:%M:%S", tm_info);
        printf("Hora actual: %s\n", buffer);
    } else {
        printf("Uso: date [-d|-t]\n");
    }
}

// Función para imprimir información del sistema
void Cmd_infosys() {
    struct utsname unameData;
    if (uname(&unameData) < 0) {
        perror("Error al obtener información del sistema");
        return;
    }
    printf("Sistema operativo: %s\n", unameData.sysname);
    printf("Nombre del nodo: %s\n", unameData.nodename);
    printf("Versión del sistema operativo: %s\n", unameData.release);
    printf("Versión del kernel: %s\n", unameData.version);
    printf("Arquitectura del hardware: %s\n", unameData.machine);
}

// Función para abrir un archivo y añadirlo a la lista de archivos abiertos
void Cmd_open(char *tr[], ListaFicheros *listaFicheros) {
    int i, df, mode = 0;

    if (tr[0] == NULL) {
        ListarFicherosAbiertos(listaFicheros);
        return;
    }

    for (i = 1; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "cr")) mode |= O_CREAT;
        else if (!strcmp(tr[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tr[i], "ro")) mode |= O_RDONLY;
        else if (!strcmp(tr[i], "wo")) mode |= O_WRONLY;
                else if (!strcmp(tr[i], "rw")) mode |= O_RDWR;
        else if (!strcmp(tr[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tr[i], "tr")) mode |= O_TRUNC;
        else break;
    }

    if ((df = open(tr[0], mode, 0666)) == -1) // Cambié el modo de permisos a 0666
        perror("Imposible abrir fichero");
    else {
        AnadirAFicherosAbiertos(listaFicheros, df, tr[0], mode);
        printf("Añadido a la tabla de ficheros abiertos: %s (modo: %d)\n", tr[0], mode);
    }
}

// Función para cerrar un archivo y eliminarlo de la lista de archivos abiertos
void Cmd_close(char *tr[], ListaFicheros *listaFicheros) {
    int df;

    if (tr[0] == NULL || (df = atoi(tr[0])) < 0) {
        ListarFicherosAbiertos(listaFicheros);
        return;
    }

    if (close(df) == -1)
        perror("Imposible cerrar descriptor");
    else {
        EliminarDeFicherosAbiertos(listaFicheros, df);
        printf("Cerrado y eliminado de la lista de ficheros abiertos: descriptor %d\n", df);
    }
}

// Función para añadir un archivo a la lista de archivos abiertos
void AnadirAFicherosAbiertos(ListaFicheros *lista, int descriptor, char *nombre, int modo) {
    Fichero *nuevo = (Fichero *)malloc(sizeof(Fichero));
    nuevo->descriptor = descriptor;
    nuevo->nombre = strdup(nombre);
    nuevo->modo = modo;
    nuevo->siguiente = lista->cabeza;
    lista->cabeza = nuevo;
}

// Función para eliminar un archivo de la lista de archivos abiertos
void EliminarDeFicherosAbiertos(ListaFicheros *lista, int descriptor) {
    Fichero *actual = lista->cabeza;
    Fichero *anterior = NULL;

    while (actual != NULL) {
        if (actual->descriptor == descriptor) {
            if (anterior == NULL)
                lista->cabeza = actual->siguiente;
            else
                anterior->siguiente = actual->siguiente;

            free(actual->nombre);
            free(actual);
            return;
        }
        anterior = actual;
        actual = actual->siguiente;
    }

    printf("Descriptor no encontrado: %d\n", descriptor);
}

// Función para listar los archivos abiertos
void ListarFicherosAbiertos(ListaFicheros *lista) {
    Fichero *actual = lista->cabeza;

    while (actual != NULL) {
        printf("Descriptor: %d, Nombre: %s, Modo: %d\n", actual->descriptor, actual->nombre, actual->modo);
        actual = actual->siguiente;
    }
}

// Función para liberar la lista de comandos
void LiberarListaComandos(Lista *lista) {
    Nodo *actual = lista->cabeza;
    Nodo *siguiente;

    while (actual != NULL) {
        siguiente = actual->siguiente;
        free(actual->comando);
        free(actual);
        actual = siguiente;
    }
}

// Función para liberar la lista de ficheros abiertos
void LiberarListaFicheros(ListaFicheros *lista) {
    Fichero *actual = lista->cabeza;
    Fichero *siguiente;

    while (actual != NULL) {
        siguiente = actual->siguiente;
        free(actual->nombre);
        free(actual);
        actual = siguiente;
    }
}