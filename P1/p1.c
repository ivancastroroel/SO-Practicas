/*
MIEMBROS
Iván Castro Roel - ivan.castro.roel@udc.es
Lucas García Boenter - l.garcia-boenter@udc.es
*/


// INCLUDE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>

// DEFINE
#define MAX_INPUT 1024
#define MAX_TROZOS 10
#define MAX_HISTORIC 100

// ESTRUCTURAS PARA EL HISTÓRICO Y FICHEROS

// - Estructura para Nodo
typedef struct Nodo {
    char comando[MAX_INPUT];
    struct Nodo *siguiente;
} Nodo;

// - Estructura para la Lista del Histórico
typedef struct {
    Nodo *cabeza;
    int tamano;
} ListaHistorico;

// - Estructura para los Ficheros que abrimos
typedef struct Fichero {
    int descriptor;
    char *nombre;
    int modo;
    struct Fichero *siguiente; // -> Puntero al siguiente nodo
} Fichero;

// - Estructura para la Lista de Ficheros que abrimos
typedef struct {
    Fichero *cabeza;
} ListaFicheros;

// ------ DECLARACIÓN DE FUNCIONES ------

// - Funciones de inicio
void imprimirPrompt();
void leerEntrada(char *entrada);
void procesarEntrada(char *entrada, ListaHistorico *historico, ListaFicheros *listaFicheros);
int TrocearCadena(char * entrada, char * trozos[]);

// - Funciones shell
void Cmd_date(char *tr[]);
void Cmd_historic(char *tr[], ListaHistorico *historico);
void AnadirComando(ListaHistorico *historico, char *comando);
void Cmd_open(char *tr[], ListaFicheros *listaFicheros);
void Cmd_close(char *tr[], ListaFicheros *listaFicheros);
void Cmd_dup(char *tr[], ListaFicheros *listaFicheros);
void Cmd_infosys();
void Cmd_help(char *tr[]);
void Cmd_makefile(char *tr[]);
void Cmd_makedir(char *tr[]);
void Cmd_listfile(char *tr[]);
void Cmd_listdir(char *tr[]);
void Cmd_erase(char *tr[]);
void Cmd_delrec(char *tr[]);

// - Funciones extra
char LetraTF (mode_t m);
mode_t ConvierteCadenaAModo(const char *permisos);
char * ConvierteModo (mode_t m, char *permisos);

//Funciones liberación de memoria
void liberarHistorico(ListaHistorico *historico);
void liberarListaFicheros(ListaFicheros *listaFicheros);

// ------ MAIN ------
int main(){
    char entrada[MAX_INPUT];
    ListaHistorico historico = {NULL, 0};
    ListaFicheros listaFicheros = {NULL};

    while (1){
        imprimirPrompt();
        leerEntrada(entrada);
        AnadirComando(&historico, entrada);  // Añadir el comando al histórico
        procesarEntrada(entrada, &historico, &listaFicheros);
    }

    // Liberamos la memoria al final
    liberarHistorico(&historico);
    liberarListaFicheros(&listaFicheros);

    return 0;
}


// ------ FUNCIONES PRINCIPALES ------

void imprimirPrompt(){
    printf("mi_shell-> ");
}

void leerEntrada(char *entrada) {
    if (fgets(entrada, MAX_INPUT, stdin) != NULL) {
        entrada[strcspn(entrada, "\n")] = 0; // Eliminamos el salto de línea
    }
}

void procesarEntrada(char * entrada, ListaHistorico *historico, ListaFicheros *listaFicheros){
    char *trozos[MAX_TROZOS];
    TrocearCadena(entrada, trozos);

    // AUTHORS
    if (strcmp(trozos[0], "authors") == 0) {
        if (trozos[1] == NULL) {
            printf("Ivan - ivan.castro.roel\n");
            printf("Lucas - l.garcia-boenter\n");
        }
        else if (strcmp(trozos[1], "-l") == 0) {
            printf("ivan.castro.roel\n");
            printf("l.garcia-boenter\n");
        }
        else if (strcmp(trozos[1], "-n") == 0) {
            printf("Ivan\n");
            printf("Lucas\n");
        }
    }
    // PID
    else if (strcmp(trozos[0], "pid") == 0) {
        printf("%d\n", getpid());
    }
    // PPID
    else if (strcmp(trozos[0], "ppid") == 0) {
        printf("%d\n", getppid());
    }
    // CD
    else if (strcmp(trozos[0], "cd") == 0) {
        char buf[1024];
        if (trozos[1] == NULL) {
            getcwd(buf, sizeof(buf));
            printf("Ruta actual:%s\n", buf);
        }
        else {
            if (chdir(trozos[1]) != 0) {
                perror("Error al cambiar el directorio");
            } else {
                getcwd(buf, sizeof(buf));
                printf("Directorio cambiado a: %s\n", buf);
            }
        }
    }
    // DATE
    else if (strcmp(trozos[0], "date") == 0) {
        Cmd_date(trozos);
    }
    // HISTORIC
    else if (strcmp(trozos[0], "historic") == 0) {
        Cmd_historic(trozos, historico);
    }
    // OPEN
    else if (strcmp(trozos[0], "open") == 0) {
        Cmd_open(trozos, listaFicheros);
    }
    // CLOSE
    else if (strcmp(trozos[0], "close") == 0) {
        Cmd_close(trozos, listaFicheros);
    }
    // DUP
    else if (strcmp(trozos[0], "dup") == 0) {
        Cmd_dup(trozos, listaFicheros);
    }
    // INFOSYS
    else if (strcmp(trozos[0], "infosys") == 0) {
        Cmd_infosys();
    }
    // HELP
    else if (strcmp(trozos[0], "help") == 0) {
        Cmd_help(trozos);
    }
    // QUIT/EXIT/BYE
    else if (strcmp(trozos[0], "quit") == 0 || strcmp(trozos[0], "exit") == 0 || strcmp(trozos[0], "bye") == 0) {
        printf("Saliendo de la shell...\n");
        exit(0);
    }
    //MAKEFILE
    else if (strcmp(trozos[0], "makefile") == 0){
        Cmd_makefile(trozos);
    }
    //MAKEDIR
    else if(strcmp(trozos[0], "makedir") == 0){
        Cmd_makedir(trozos);
    }

    //LISTFILE -long -acc -link 
    else if(strcmp(trozos[0], "listfile") == 0){
        Cmd_listfile(trozos);
    }
    //CWD
    else if(strcmp(trozos[0], "cwd") == 0){
        char buf[1024];
        if (getcwd(buf, sizeof(buf)) != NULL) {
            printf("Directorio actual: %s\n", buf);
        } else {
            perror("Error obteniendo el directorio de trabajo actual");
        }
    }
    //LISTDIR
    else if(strcmp(trozos[0], "listdir") == 0){
        Cmd_listdir(trozos);
    }
    //ERASE
    else if(strcmp(trozos[0], "erase") == 0){
        Cmd_erase(trozos);
    }
    //ERASE RECURSIVO
    else if(strcmp(trozos[0], "delrec") == 0){
        Cmd_delrec(trozos);
    }
    else {
        printf("\nComando no reconocido\n");
    }
}


// ------ FUNCIONES EXTRA ------

int TrocearCadena(char * cadena, char * trozos[]) {
    int i = 1;

    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL)
        return 0;

    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL)
        i++;

    return i;
}

char LetraTF (mode_t m)
{
     switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
        case S_IFSOCK: return 's'; /*socket */
        case S_IFLNK: return 'l'; /*symbolic link*/
        case S_IFREG: return '-'; /* fichero normal*/
        case S_IFBLK: return 'b'; /*block device*/
        case S_IFDIR: return 'd'; /*directorio */ 
        case S_IFCHR: return 'c'; /*char device*/
        case S_IFIFO: return 'p'; /*pipe*/
        default: return '?'; /*desconocido, no deberia aparecer*/
     }
}

mode_t ConvierteCadenaAModo(const char *permisos) {
    mode_t modo = 0;

    // Propietario (user)
    if (permisos[1] == 'r') modo |= S_IRUSR;  // Lectura del propietario
    if (permisos[2] == 'w') modo |= S_IWUSR;  // Escritura del propietario
    if (permisos[3] == 'x') modo |= S_IXUSR;  // Ejecución del propietario

    // Grupo (group)
    if (permisos[4] == 'r') modo |= S_IRGRP;  // Lectura del grupo
    if (permisos[5] == 'w') modo |= S_IWGRP;  // Escritura del grupo
    if (permisos[6] == 'x') modo |= S_IXGRP;  // Ejecución del grupo

    // Otros (others)
    if (permisos[7] == 'r') modo |= S_IROTH;  // Lectura de otros
    if (permisos[8] == 'w') modo |= S_IWOTH;  // Escritura de otros
    if (permisos[9] == 'x') modo |= S_IXOTH;  // Ejecución de otros

    return modo;
}

char * ConvierteModo (mode_t m, char *permisos)
{
    strcpy (permisos,"---------- ");
    
    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';    /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';    /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';    /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';    /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';
    
    return permisos;
}




// ------ FUNCIONES SHELL ------

// Función para el comando "date"
void Cmd_date(char *tr[]) {
    time_t t;
    struct tm *tm_info;
    char buffer[80];

    time(&t);
    tm_info = localtime(&t);

    if (tr[1] == NULL) {
        strftime(buffer, 80, "%d/%m/%Y %H:%M:%S", tm_info);
        printf("Fecha y hora actual: %s\n", buffer);
    } else if (strcmp(tr[1], "-d") == 0) {
        strftime(buffer, 80, "%d/%m/%Y", tm_info);
        printf("Fecha actual: %s\n", buffer);
    } else if (strcmp(tr[1], "-t") == 0) {
        strftime(buffer, 80, "%H:%M:%S", tm_info);
        printf("Hora actual: %s\n", buffer);
    } else {
        printf("Uso: date [-d|-t]\n");
    }
}

// Función para manejar el histórico de comandos
void Cmd_historic(char *tr[], ListaHistorico *historico) {
    Nodo *actual = historico->cabeza;
    int contador = 0;

    if (tr[1] == NULL) {  // Mostrar todo el histórico
        while (actual != NULL) {
            printf("%d: %s", contador++, actual->comando);
            actual = actual->siguiente;
        }
    } else if (tr[1][0] == '-') {  // Mostrar últimos N comandos
        int N = atoi(tr[1] + 1); 
        actual = historico->cabeza;
        while (actual != NULL && contador < N) {
            printf("%d: %s", contador++, actual->comando);
            actual = actual->siguiente;
        }
    } else {  // Repetir comando N
        int N = atoi(tr[1]);
        actual = historico->cabeza;
        while (actual != NULL && contador < N) {
            actual = actual->siguiente;
            contador++;
        }
        if (actual != NULL) {
            printf("Repitiendo comando: %s\n", actual->comando);
            procesarEntrada(actual->comando, historico, NULL);
        }
    }
}

// Función para añadir un comando al histórico
void AnadirComando(ListaHistorico *historico, char *comando) {
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    strcpy(nuevo->comando, comando);
    nuevo->siguiente = historico->cabeza;
    historico->cabeza = nuevo;
    historico->tamano++;
    free(nuevo);
}

// Función para el comando "open"
void Cmd_open(char *tr[], ListaFicheros *listaFicheros) {
    Fichero *actual = listaFicheros->cabeza;
    if (tr[1] == NULL) {
        // Listar archivos abiertos
        printf("Lista de ficheros abiertos...\n");
        // Código para listar
        while(actual != NULL){
            printf("Name: %s, Mode: %d, Descriptor: %d\n", actual->nombre, actual->modo, actual->descriptor);
            actual = actual->siguiente;
        }
    } else {
        // Proceso de apertura de archivo
        int modo = 0;
        char *nombreArchivo = tr[1];

        // Mapeo de los modos desde el segundo parámetro del comando
        if(tr[2] != NULL){
            if (strcmp(tr[2], "ro") == 0) modo = O_RDONLY;
            else if (strcmp(tr[2], "wo") == 0) modo = O_WRONLY;
            else if (strcmp(tr[2], "rw") == 0) modo = O_RDWR;
            else if (strcmp(tr[2], "cr") == 0) modo = O_CREAT | O_WRONLY;
            else if (strcmp(tr[2], "ap") == 0) modo = O_APPEND;
            else if (strcmp(tr[2], "tr") == 0) modo = O_TRUNC | O_WRONLY;
            else if (strcmp(tr[2], "ex") == 0) modo = O_EXCL | O_CREAT;
            else {
                printf("Modo desconocido: %s\n", tr[2]);
                return;
            }
        }
        else{
            modo = O_RDONLY; // Modo por defecto
        }

        // Código para abrir un archivo
        int fd = open(nombreArchivo, modo);
        if (fd == -1) {
            perror("Error al abrir el archivo");
        } 
        else {
            printf("Archivo abierto: %s (descriptor: %d)\n", nombreArchivo, fd);

            //Creamos un nuevo nodo para la lista de archivos abiertos
            Fichero *nuevoFichero = (Fichero *)malloc(sizeof(Fichero));
            nuevoFichero->nombre = strdup(nombreArchivo);
            nuevoFichero->descriptor = fd;
            nuevoFichero->modo = modo;
            nuevoFichero->siguiente = listaFicheros->cabeza;

            // Añadimos el nuevo nodo al inicio de la lista
            listaFicheros->cabeza = nuevoFichero;   
        }
    }
}

// Función para el comando "close"
void Cmd_close(char *tr[], ListaFicheros *listaFicheros) {
    if (tr[1] == NULL) {
        printf("Debe especificar un descriptor de fichero.\n");
        return;
    }
    
    int fd = atoi(tr[1]);  // Convertir el argumento a entero
    Fichero *actual = listaFicheros->cabeza;
    Fichero *anterior = NULL;

    // Buscar el archivo en la lista
    while (actual != NULL && actual->descriptor != fd) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        printf("Descriptor %d no encontrado.\n", fd);
        return;
    }

    // Cerrar el archivo
    if (close(fd) == -1) {
        perror("Error al cerrar el archivo");
    } else {
        printf("Descriptor %d cerrado.\n", fd);

        // Eliminar el archivo de la lista
        if (anterior == NULL) {
            listaFicheros->cabeza = actual->siguiente;
        } else {
            anterior->siguiente = actual->siguiente;
        }
        free(actual->nombre);
        free(actual);
    }
}

// Función para el comando "dup"
void Cmd_dup(char *tr[], ListaFicheros *listaFicheros) {
    if (tr[1] == NULL) {
        printf("Debe especificar un descriptor de fichero.\n");
        return;
    }
    
    int fd = atoi(tr[1]);  // Convertir el argumento a entero
    int nuevo_fd = dup(fd);  // Duplicar el descriptor de archivo

    if (nuevo_fd == -1) {
        perror("Error al duplicar el archivo");
    } else {
        printf("Descriptor %d duplicado. Nuevo descriptor: %d\n", fd, nuevo_fd);

        // Buscar el archivo original en la lista
        Fichero *actual = listaFicheros->cabeza;
        while (actual != NULL && actual->descriptor != fd) {
            actual = actual->siguiente;
        }

        // Añadir el nuevo descriptor a la lista
        if (actual != NULL) {
            Fichero *nuevoFichero = (Fichero *)malloc(sizeof(Fichero));
            nuevoFichero->nombre = strdup(actual->nombre);
            nuevoFichero->descriptor = nuevo_fd;
            nuevoFichero->modo = actual->modo;
            nuevoFichero->siguiente = listaFicheros->cabeza;
            listaFicheros->cabeza = nuevoFichero;
        }
    }
}

// Función para el comando "infosys"
void Cmd_infosys() {
    struct utsname info_sistema;
    
    if (uname(&info_sistema) == -1) {
        perror("Error al obtener la información del sistema");
    } else {
        printf("Sistema: %s\n", info_sistema.sysname);
        printf("Nodo: %s\n", info_sistema.nodename);
        printf("Release: %s\n", info_sistema.release);
        printf("Versión: %s\n", info_sistema.version);
        printf("Máquina: %s\n", info_sistema.machine);
    }
}

// Función para el comando "help"
void Cmd_help(char *tr[]) {
    if (tr[1] == NULL) {
        printf("Comandos disponibles:\n");
        printf("  authors\n");
        printf("  cd\n");
        printf("  close\n");
        printf("  date\n");
        printf("  dup\n");
        printf("  help\n");
        printf("  historic\n");
        printf("  infosys\n");
        printf("  open\n");
        printf("  pid\n");
        printf("  ppid\n");
        printf("  quit\n");
        printf("  exit\n");
        printf("  bye\n");
    } else {
        if (strcmp(tr[1], "authors") == 0) {
            printf("authors [-l|-n]: Muestra los autores del proyecto.\n");
        } else if (strcmp(tr[1], "cd") == 0) {
            printf("cd [directorio]: Cambia el directorio actual.\n");
        } else if (strcmp(tr[1], "close") == 0) {
            printf("close [df]: Cierra el descriptor de archivo indicado.\n");
        } else if (strcmp(tr[1], "date") == 0) {
            printf("date [-d|-t]: Muestra la fecha (-d) o la hora (-t) actual.\n");
        } else if (strcmp(tr[1], "dup") == 0) {
            printf("dup [df]: Duplica el descriptor de archivo.\n");
        } else if (strcmp(tr[1], "historic") == 0) {
            printf("historic [N|-N]: Muestra o repite comandos del histórico.\n");
        } else if (strcmp(tr[1], "infosys") == 0) {
            printf("infosys: Muestra información del sistema.\n");
        } else if (strcmp(tr[1], "open") == 0) {
            printf("open [file] [modo]: Abre un archivo con el modo especificado.\n");
        } else if (strcmp(tr[1], "pid") == 0) {
            printf("pid: Muestra el PID del proceso actual.\n");
        } else if (strcmp(tr[1], "ppid") == 0) {
            printf("ppid: Muestra el PPID del proceso padre.\n");
        } else if (strcmp(tr[1], "quit") == 0 || strcmp(tr[1], "exit") == 0 || strcmp(tr[1], "bye") == 0) {
            printf("quit/exit/bye: Finaliza la shell.\n");
        } else {
            printf("Comando no reconocido. Usa 'help' para ver los comandos disponibles.\n");
        }
    }
}

//Función para el comando "makefile"
void Cmd_makefile(char *trozos[]){
    int result =  2;
    if (trozos[1] == NULL){
        perror("Falta el nombre del archivo");
    }
    else if(trozos[2] == NULL && trozos[1] != NULL){
        result = creat(trozos[1], S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
    }
    else{
        mode_t mode = ConvierteCadenaAModo(trozos[2]);
        result = creat(trozos[1], mode);
    }

    if (result == -1) {
        perror("Error al crear el archivo");
    } else{
        printf("Archivo creado con éxito \n");
    }
}

//Función para el comando "makedir"
void Cmd_makedir(char *trozos[]){

    int result = 2;
    if (trozos[1] == NULL){
        perror("Falta el nombre del directorio");
    }
    else if(trozos[2] == NULL && trozos[1] != NULL){
        result = mkdir(trozos[1], 0755);
    }
    else{
        mode_t mode = ConvierteCadenaAModo(trozos[2]);
        result = mkdir(trozos[1], mode);
    }

    if (result == -1) {
        perror("Error al crear el archivo");
    } else if(result == 0) {
        printf("Archivo creado con éxito \n");
    }
}

//Función para el comando "listfile"
void Cmd_listfile(char *trozos[]){
    struct stat fileStat;
    char permisos[15];
    char timebuf[100]; // Para almacenar la fecha y hora
    struct tm *tm_info;

    // Verificamos si se ha pasado el nombre del archivo y asignamos filename
    char *filename = (trozos[2] != NULL) ? trozos[2] : trozos[1];

    if (filename == NULL) {
        printf("Error: No se ha proporcionado el nombre del archivo.\n");
        return;
    }

    // Obtenemos la información del archivo
    if (stat(filename, &fileStat) == -1) {
        perror("Error al obtener información del archivo");
        return;
    }else{
        if(trozos[2] == NULL){ //si no pasamos ninguna opción entonces muestra num-inodo y nombre
            // 1. Número de inodo (st_ino)
            printf("%lu\t", (unsigned long)fileStat.st_ino);
            // 2. Nombre del archivo
            printf("%s\n", filename);
        }else {
            // -long muestra -> 2024/10/02-12:12   1 (26211315)     ivan   staff -rwxr-xr-x   101960 shell-macos.out
            if(strcmp(trozos[1], "-long") == 0){
                // 1. Fecha de modificación
                tm_info = localtime(&fileStat.st_mtime);
                if (tm_info == NULL) {
                    perror("Error al convertir la hora");
                    return;
                }
                strftime(timebuf, sizeof(timebuf), "%Y/%m/%d-%H:%M", tm_info);
                printf("%s\t", timebuf);

                // 2. Número de enlaces (st_nlink)
                printf("%hu\t", fileStat.st_nlink);  // nlink_t es unsigned short

                // 3. Tamaño del archivo (st_size)
                printf("(%lld)\t", (long long)fileStat.st_size);

                // 4. Propietario (st_uid)
                struct passwd *pw = getpwuid(fileStat.st_uid);
                if (pw) {
                    printf("%s\t", pw->pw_name);
                } else {
                    printf("%d\t", fileStat.st_uid);
                }

                // 5. Grupo (st_gid)
                struct group *gr = getgrgid(fileStat.st_gid);
                if (gr) {
                    printf("%s\t", gr->gr_name);
                } else {
                    printf("%d\t", fileStat.st_gid);
                }

                // 6. Permisos (st_mode)
                printf("%s\t", ConvierteModo(fileStat.st_mode, permisos));

                // 7. Número de inodo (st_ino)
                printf("%lu\t", (unsigned long)fileStat.st_ino);

                // 8. Nombre del archivo
                printf("%s\n", filename);
            }
            // -acc muestra -> 101960  2024/10/09-20:18 shell-macos.out
            else if(strcmp(trozos[1], "-acc") == 0){
                // 1. Número de inodo (st_ino)
                printf("%lu\t", (unsigned long)fileStat.st_ino);
                // 2. Fecha de modificación
                tm_info = localtime(&fileStat.st_mtime);
                if (tm_info == NULL) {
                    perror("Error al convertir la hora");
                    return;
                }
                strftime(timebuf, sizeof(timebuf), "%Y/%m/%d-%H:%M", tm_info);
                printf("%s\t", timebuf);
                // 3. Nombre del archivo
                printf("%s\n", filename);
            }
            // -link muestra -> 101960  shell-macos.out
            else if(strcmp(trozos[1], "-link") == 0){
                // 1. Número de inodo (st_ino)
                printf("%lu\t", (unsigned long)fileStat.st_ino);
                // 2. Verificamos si es un enlace simbólico
                if (S_ISLNK(fileStat.st_mode)) {
                    char link_target[PATH_MAX];
                    ssize_t len = readlink(filename, link_target, sizeof(link_target) - 1);
                    if (len != -1) {
                        link_target[len] = '\0';  // Null-terminate la cadena
                        printf("%s -> %s\n", filename, link_target);
                    } else {
                        perror("Error leyendo enlace simbólico");
                    }
                } else {
                    // Si no es un enlace simbólico, solo mostramos el nombre del archivo
                    printf("%s\n", filename);
                }
            }
        }
    }
}

//Función para el comando "listdir"
void Cmd_listdir(char *trozos[]){
    if(trozos[1]==NULL){
            perror("No hay directorio");
        }
        struct dirent *entradadir;  // Estructura para almacenar la entrada del directorio
        DIR *directorio = opendir(trozos[1]);  // Abre el directorio

        if (directorio == NULL) {
            perror("Error al abrir el directorio");  // Imprime un mensaje de error si no se puede abrir
            return;
        }

        // Lee cada entrada en el directorio
        while ((entradadir = readdir(directorio)) != NULL) {
            // Ignora las entradas "." y ".."
            if (strcmp(entradadir->d_name, ".") != 0 && strcmp(entradadir->d_name, "..") != 0) {
                printf("%s\n", entradadir->d_name);  // Imprime el nombre de la entrada
            }
        }

        closedir(directorio);  // Cierra el directorio después de leerlo
}

//Función para el comando "erase"
void Cmd_erase(char *trozos[]) {
    if (trozos[1] == NULL) {
        printf("No se han especificado archivos o directorios para borrar.\n");
        return;
    }

    for (int i = 1; trozos[i] != NULL; i++) { // Comenzamos desde 1 para omitir el nombre del comando
        
        // Verificamos si es un archivo o directorio
        struct stat st;
        if (stat(trozos[i], &st) == -1) {
            perror("Error al acceder al archivo o directorio");
            continue; // Continuar con el siguiente nombre
        }

        // Borrar el archivo
        if (S_ISREG(st.st_mode)) {
            if (remove(trozos[i]) == 0) {
                printf("Archivo '%s' eliminado con éxito.\n", trozos[i]);
            } else {
                perror("Error al eliminar el archivo");
            }
        } 
        // Borrar el directorio
        else if (S_ISDIR(st.st_mode)) {
            DIR *dir = opendir(trozos[i]);
            if (!dir) {
                perror("Error al abrir el directorio");
                return;
            }

            // Contar las entradas en el directorio
            struct dirent *entradas;
            int isEmpty = 1;  // Asumimos que está vacío

            while ((entradas = readdir(dir)) != NULL) {
                if (strcmp(entradas->d_name, ".") != 0 && strcmp(entradas->d_name, "..") != 0) {
                    isEmpty = 0;  // Encontramos algo que no es . o ..
                    break;
                }
            }
            closedir(dir);

            // Si está vacío, eliminar el directorio
            if (isEmpty) {
                if (rmdir(trozos[i]) == 0) {
                    printf("Directorio %s eliminado correctamente.\n", trozos[i]);
                } else {
                    perror("Error al eliminar el directorio");
                }
            } else {
                printf("El directorio %s no está vacío y no se puede eliminar.\n", trozos[i]);
            }
        }

        // Si no es un archivo regular ni un directorio
        else {
            printf("'%s' no es un archivo regular ni un directorio.\n", trozos[i]);
        }
    }
}

void Cmd_delrec(char *trozos[]){

}

// ----- FUNCIONES PARA LIBERAR MEMORIA -------

// liberamos la memoria del histórico
void liberarHistorico(ListaHistorico *historico) {
    Nodo *actual = historico->cabeza;
    while (actual != NULL) {
        Nodo *temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    historico->cabeza = NULL;  // Opcional: para evitar un puntero colgante
}

// liberamos la memoria de la lista de ficheros abiertos
void liberarListaFicheros(ListaFicheros *listaFicheros) {
    Fichero *actual = listaFicheros->cabeza;
    while (actual != NULL) {
        Fichero *temp = actual;
        actual = actual->siguiente;
        free(temp->nombre);  // Liberar el nombre
        free(temp);          // Liberar el nodo
    }
    listaFicheros->cabeza = NULL;  // Opcional
}