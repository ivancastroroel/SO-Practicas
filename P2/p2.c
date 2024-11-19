#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include <features.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include "list.h"
#include "memoria.h"
#include "listaMemoria.h"
#include "listaProcesos.h"


void Bucle(bool finish, char *env[]);
void printPrompt();
void leerEntrada(command *comando, List *listaHistorial); // pasamos una string por referencia para guardar sus datos
void procesarEntrada(command *peticion, List *listaHistorial, List *listaFicheros, ListM *listaMemoria, bool *end, char *env[], ListP *listaProcesos);
void commands(int argument, List *listaHistorial, List *listaFicheros, ListM *listaMemoria, bool *end, char *argument1, char *argument2, char *argument3, command argumentos, char *env[], ListP *listaProcesos);
int chooseCommand(char *argument1, char *argument2, char *argument3);
void date();
void dateD();
void dateT();
void historic(List listaHistorial);
void historicN(List listaHistorial, int n);
bool borraLista(List *L);
void commandoN(List listaHistorial, List *listaFicheros, ListM *listaMemoria, int n, bool *end, char *env[], ListP *listaProcesos);
void infosys();
void cdDirActual();
bool cambiarDirectorio(const char *pathname);
void openFile(char *tr[], List *listaFicheros);
void listOpen(List listaFicheros);
void inicializarListaFicheros(List *listaFicheros);
bool closeDf(List *listaFicheros, int df);
void dupDf(List *listaFicheros, int df);
void help(char *cadena[]);
char LetraTF(mode_t m);
char *ConvierteModo2(mode_t m);
void createDir(char *name);
void createFile(char *name);
void delete(char *name);
void deleteTree(char *name);
void FileDirInformation(command argumentos);
void listContent(command argumentos, bool longg, bool recb, bool reca, bool hid, bool link, bool acc);

// MAIN
int main(int argc, char *argv[], char *env[])
{
    bool end = false; // inicializamos la variable para mantener el bucle

    Bucle(end, env);

    return 0;
}

// BUCLE PRINCIPAL
void Bucle(bool finish, char *env[])
{
    command entrada;
    List historial, ficherosAbiertos;
    ListM listaMemoria;
    ListP listaProcesos;

    createEmptyList(&historial);        // crea una lista vacía, es decir == NULL
    createEmptyList(&ficherosAbiertos); // " & " se usa para obtener la dirección de memoria de la variable, no su valor
    createEmptyListM(&listaMemoria);
    createEmptyListP(&listaProcesos);
    inicializarListaFicheros(&ficherosAbiertos);

    while (!finish)
    {
        printPrompt();
        leerEntrada(&entrada, &historial);
        procesarEntrada(&entrada, &historial, &ficherosAbiertos, &listaMemoria, &finish, env, &listaProcesos);
    }
    free(historial);
    free(ficherosAbiertos);
    free(listaMemoria);
}

// FUNCIONES PARA TRABAJAR CON LOS COMANDOS

//
void printPrompt()
{
    printf("-> ");
}

void leerEntrada(command *comando, List *listaHistorial)
{
    Item data;
    char *trozos[COMMAND_LENGTH];

    fgets(*comando, COMMAND_LENGTH, stdin); // guardamos la entrada del usuario en el string comando, pasado por referencia para guardar su contenido

    strcpy(data.name, *comando);
    insertItem(data, listaHistorial); // inserta el comando en el Historial(Lista)

    trocearCadena(data.name, trozos);

    if (strcmp(trozos[0], "command") == 0)
        deleteAtPosition(last(*listaHistorial), listaHistorial);
}

void procesarEntrada(command *peticion, List *listaHistorial, List *listaFicheros, ListM *listaMemoria, bool *end, char *env[], ListP *listaProcesos)
{
    command temp;
    char *trozo[COMMAND_LENGTH]; // creamos los parámetros donde guardaremos los datos de la petición, y un array de punteros nulos para comparar en la función trocearCadena
    int palabras;
    strcpy(temp, *peticion);
    palabras = trocearCadena(*peticion, trozo);

    if (palabras == 0)
        return;

    commands(chooseCommand(trozo[0], trozo[1], trozo[2]), listaHistorial, listaFicheros, listaMemoria, end, trozo[0], trozo[1], trozo[2], temp, env, listaProcesos);
}

void inicializarListaFicheros(List *listaFicheros) // metemos los descriptores de archivos reservados en la lista
{
    Item d;
    d.posicion = 0;

    d.fileDescriptor = STDIN_FILENO;
    strcpy(d.name, "entrada estandar");
    d.mode |= fcntl(d.fileDescriptor, F_GETFL); // fcntl nos devuelve el modo de apertura de un fichero (con el flag F_GETFL) dado un descriptor de fichero
    insertItem(d, listaFicheros);

    d.fileDescriptor = STDOUT_FILENO;
    strcpy(d.name, "salida estandar");
    d.mode |= fcntl(d.fileDescriptor, F_GETFL);
    insertItem(d, listaFicheros);

    d.fileDescriptor = STDERR_FILENO;
    strcpy(d.name, "error estandar");
    d.mode |= fcntl(d.fileDescriptor, F_GETFL);
    insertItem(d, listaFicheros);
}

int chooseCommand(char *argument1, char *argument2, char *argument3) // comprobamos que comando hemos pasado
{
    if (argument2 != NULL && (!strcmp(argument2, "-?") || !strcmp(argument2, "-help")) && argument3 == NULL)
        return 27;
    if (strcmp(argument1, "authors") == 0) // authors
    {
        if (argument2 == NULL)
            return 1;
        else if (strcmp(argument2, "-l") == 0 && argument3 == NULL)
            return 2;
        else if (strcmp(argument2, "-n") == 0 && argument3 == NULL)
            return 3;
    }

    else if (strcmp(argument1, "date") == 0 && argument2 == NULL) // date
    {
        if (argument2 == NULL)
        {
            return 4;
        }
        else if (strcmp(argument2, "-d") == 0 && argument3 == NULL)
            return 5;
        else if (strcmp(argument2, "-t") == 0 && argument3 == NULL)
            return 6;
    }

    else if (strcmp(argument1, "historic") == 0) // historic
    {
        if (argument2 == NULL)
            return 7;
        else if (strcmp(argument2, "-c") == 0 && argument3 == NULL)
            return 8;
        else if ((argument2[0] == '-N' && isdigit(argument2[1])) && argument3 == NULL)
            return 9;
        else if ((argument2[0] == 'N' && isdigit(argument2[1])) && argument3 == NULL)
            return 10;
    }

    else if ((strcmp(argument1, "quit") == 0 || strcmp(argument1, "bye") == 0 || strcmp(argument1, "exit") == 0) && argument2 == NULL) // quit, bye, exit
        return 11;

    else if (strcmp(argument1, "pid") == 0) // pid
    {
        if (argument2 == NULL)
            return 12;
        else if (strcmp(argument2, "-p") == 0 && argument3 == NULL)
            return 13;
    }

    else if (strcmp(argument1, "infosys") == 0 && argument2 == NULL) // infosys
        return 14;

    else if (strcmp(argument1, "cd") == 0) // cd[dir]
    {
        if (argument2 == NULL)
            return 15;
        else if (argument3 == NULL)
            return 16;
    }

    else if (strcmp(argument1, "open") == 0) // open file mode
    {
        if (argument2 == NULL) // en caso de ser solo "open", mostramos la lista (return entero de la función openLIST)
            return 17;
        else
            return 18;
    }

    else if (strcmp(argument1, "listopen") == 0 && argument2 == NULL) // listopen
        return 19;

    else if (strcmp(argument1, "close") == 0) // close
    {
        if (argument2 == NULL)
            return 20;
        else if (isdigit(argument2[0]) && argument3 == NULL)
            return 21;
    }

    if (strcmp(argument1, "dup") == 0) // dup
    {
        if (argument2 == NULL)
            return 16;
        else if (isdigit(argument2[0]))
            return 18;
    }

    else if (strcmp(argument1, "date") == 0 && argument2 == NULL) // date
        return 19;

    else if (strcmp(argument1, "help") == 0) // help
        return 20;

    else if (strcmp(argument1, "create") == 0)
    {
        if (argument2 == NULL)
            return 13;
        else if (argument3 == NULL)
            return 21;
        else if (strcmp(argument2, "-f") == 0)
            return 22;
    }

    else if (strcmp(argument1, "delete") == 0)
    {
        if (argument2 != NULL && argument3 == NULL)
            return 23;
        else
            return 13;
    }

    else if (strcmp(argument1, "deltree") == 0)
    {
        if (argument2 != NULL && argument3 == NULL)
            return 24;
        else
            return 13;
    }

    else if (strcmp(argument1, "list") == 0)
    {
        if (argument2 != NULL)
            return 25;
        else
            return 13;
    }

    else if (strcmp(argument1, "stat") == 0)
    {
        if (argument2 == NULL)
            return 13;
        else
            return 26;
    }

    else if (!strcmp(argument1, "malloc"))
    {
        if (argument2 == NULL)
            return 29;
        else if (!strcmp(argument2, "-free"))
            return 30;
        else
            return 28;
    }

    else if (!strcmp(argument1, "mem"))
        return 31;

    else if (!strcmp(argument1, "mmap"))
        return 32;

    else if (!strcmp(argument1, "read"))
        return 33;

    else if (!strcmp(argument1, "write"))
        return 34;

    else if (!strcmp(argument1, "memfill"))
        return 35;

    else if (!strcmp(argument1, "memdump"))
        return 36;

    else if (!strcmp(argument1, "recurse"))
        return 37;

    else if (argument1 != NULL)
        return 0;

    return 99999;
}

void commands(int argument, List *listaHistorial, List *listaFicheros, ListM *listaMemoria, bool *end, char *argument1, char *argument2, char *argument3, command argumentos, char *env[], ListP *listaProcesos)
{
    switch (argument)
    {
    case 1: // authors
        printf("Author1: ivan.castro.roel@udc.es\nAuthor2: l.garcia-boenter@udc.es\n");
        break;

    case 2: // authors -l
        printf("ivan.castro.roel@udc.es\nl.garcia-boenter@udc.es\n");
        break;

    case 3: // authors -n
        printf("Ivan\nLucas\n");
        break;

    case 4: // date
        date();
        break;

    case 5: // date -d
        dateD();
        break;

    case 6: // date -t
        dateT();
        break;

    case 7: // hist
        historic(*listaHistorial);
        break;

    case 8: // hist -c
        borraLista(listaHistorial);
        break;

    case 9: // hist -n
        argument2[0] = '0';
        historicN(*listaHistorial, atoi(argument2));
        break;

    case 10: // historic N
        commandoN(*listaHistorial, listaFicheros, listaMemoria, atoi(argument2), end, env, listaProcesos);
        break;

    case 11: //  quit, bye, exit 
        (*end) = true;
        borraLista(listaFicheros);
        borraLista(listaHistorial);
        borraListaM(listaMemoria);
        break;

    case 12: // pid
        printf("Pid de shell: %d\n", getpid());
        break;

    case 13: // pid -p
        printf("Pid del padre del shell: %d\n", getppid());
        break;

    case 14: // infosys
        infosys();
        break;

    case 15: // chdir mostrar
        cdDirActual();
        break;

    case 16: // chdir entrar
        cambiarDirectorio(argument2);
        break;

    case 17: // listOpen
        listOpen(*listaFicheros);
        break;

    case 18: // open [file] [mode]
        openFile(&argumentos, listaFicheros);
        break;

    case 19: // close [df]
        closeDf(listaFicheros, atoi(argument2));
        break;

    case 20: // dup
        dupDf(listaFicheros, atoi(argument2));
        break;

    case 21: // help
        help(&argument2);
        break;

    case 22: // create dir
        createDir(argument2);
        break;

    case 23: // create file
        createFile(argument3);
        break;

    case 24: // delete
        delete (argument2);
        break;

    case 25: // deltree
        deleteTree(argument2);
        break;

    case 26: // list
        listContent(argumentos, false, false, false, false, false, false);
        break;

    case 27: // stat
        FileDirInformation(argumentos);
        break;

    case 28: // help -?
        help(&argument1);
        break;

    case 29: // malloc [size]
        mallocMemoria(atoi(argument2), listaMemoria);
        break;

    case 30: // malloc a secas
        printListM(*listaMemoria, "malloc");
        break;

    case 31: // malloc free
        if (argument3 == NULL)
            printListM(*listaMemoria, "malloc");
        else
            freeMalloc("malloc", listaMemoria, atoi(argument3));
        break;

    case 32: // mem
        if (argument2 == NULL || !strcmp(argument2, "-all"))
        {
            memVars();
            memFuncs();
            printListM(*listaMemoria, " ");
        }
        else if (!strcmp(argument2, "-pmap"))
            Do_MemPmap();
        else if (!strcmp(argument2, "-vars"))
            memVars();
        else if (!strcmp(argument2, "-funcs"))
            memFuncs();
        else if (!strcmp(argument2, "-blocks"))
            printListM(*listaMemoria, "all");

        break;

    case 33: // mmap
        if (argument2 == NULL || (!strcmp(argument2, "-free") && argument3 == NULL))
            printListM(*listaMemoria, "mmaped");
        else if (!strcmp(argument2, "-free") && argument3 != NULL)
            mmapFree(argument3, listaMemoria);
        else
            CmdMmap(argument2, argument3, listaMemoria);
        break;

    case 34: // read
        if (argument2 == NULL)
            printf("Faltan parametros");
        else
            LeerFichero(argument2, argument3, argumentos);
        break;

    case 35: // write
        EscribirFichero(argumentos);
        break;

    case 36: // memfill
        LlenarMemoria(argumentos);
        break;

    case 37: // memdump
        memDump(argumentos);
        break;

    case 38: // recurse
        Recursiva(atoi(argument2));
        break;

    default:
        printf("Invalid command\n");
        break;
    }
}

//------ FUNCIONES ------

void date() // date
{
    time_t tiempo;
    tiempo = time(NULL);

    struct tm fecha = *localtime(&tiempo);
    printf("%02d/%02d/%d\n%02d:%02d:%02d\n", fecha.tm_mday, fecha.tm_mon + 1, fecha.tm_year + 1900,
           fecha.tm_hour, fecha.tm_min, fecha.tm_sec);
}

void dateD() // date -d
{
    time_t tiempo;
    tiempo = time(NULL);

    struct tm fecha = *localtime(&tiempo);
    printf("%02d/%02d/%d\n", fecha.tm_mday, fecha.tm_mon + 1, fecha.tm_year + 1900);
}

void dateT() // date -t
{
    time_t tiempo = time(NULL);

    struct tm *tmStruct = localtime(&tiempo);
    printf("%02d:%02d:%02d\n", tmStruct->tm_hour, tmStruct->tm_min, tmStruct->tm_sec);
}

void historic(List listaHistorial) // historic
{
    Pos p;
    int i = 0;

    for (p = listaHistorial; (p != NULL); p = p->next) // recorremos el historial y mostramos su contenido
    {
        printf("%d -> %s", i, p->data.name);
        i++;
    }
}

void historicN(List listaHistorial, int n) // historic -N
{
    Pos p;
    int i = 0;
    p = listaHistorial;

    while (p != NULL && i < n) //  enseña el historial hasta llegar al elemento anterior de la posición dada
    {
        printf("%d -> %s", i, p->data.name);
        p = p->next;
        i++;
    }
}

void commandoN(List listaHistorial, List *listaFicheros, ListM *listaMemoria, int n, bool *end, char *env[], ListP *listaProcesos) // historic N
{
    Pos p;
    command temp, aux;
    char *trozo[COMMAND_LENGTH];

    if (isEmptyList(listaHistorial))
    {
        printf("El historial está vacío\n");
        return;
    }

    p = findPosition(n, listaHistorial); // conseguimos el elemento de la listaHistorial de la posición que nos interesa

    if (p == NULL || p->data.posicion != n) // si n es una posición que no existe en la listaHistorial entonces acaba
    {
        printf("No hay elemento %d en el historico\n", n);
        return;
    }

    strcpy(temp, p->data.name);
    printf("Ejecutando hist (%d): %s", n, temp); // en caso de que sí haya esa posición printeamos la muletilla

    strcpy(aux, p->data.name); // copiamos en temp el nombre del comando para no desconfigurar la listaHistorial

    trocearCadena(temp, trozo); // troceamos la cadena y obtenemos en el array de punteros trozo el comando separado
    commands(chooseCommand(trozo[0], trozo[1], trozo[2]), &listaHistorial, listaFicheros, listaMemoria, end, trozo[0], trozo[1], trozo[2], aux, env, listaProcesos);
}

bool borraLista(List *L) // historic -c
{
    Pos p, temp;
    p = *L;

    if (*L == NULL)
        return false;

    while (p != NULL) // mientras haya elementos en la listaHistorial
    {
        temp = p;    // guardamos en temp el elemento de la listaHistorial
        p = p->next; // p pasa al siguiente elemento para que cuando liberemos el espacio no nos quedemos fuera de la listaHistorial
        free(temp);  // liberamos el espacio
    }
    createEmptyList(L); // inicializamos de vuelta la listaHistorial
    return true;
}

void infosys() // infosys
{
    struct utsname unameData; // definimos un registro tipo utsname

    uname(&unameData); // llamamos a la función uname con un puntero al registro, ahora deberíamos tener la información del sistema en el registro
    printf("%s (%s), OS: %s-%s-%s\n", unameData.nodename, unameData.machine, unameData.sysname, unameData.release, unameData.version);
}

void cdDirActual() // cd
{
    char path[200]; // creamos un string donde guardaremos el camino del directorio actual

    getcwd(path, sizeof(path)); // guardamos en path el directorio actual con getcwd y le asignamos el tamaño de path
    printf("%s\n", path);
}

bool cambiarDirectorio(const char *pathname) // cd [dir]
{
    if (chdir(pathname) == -1)
    {
        perror("Imposible cambiar directorio");
        return false;
    }
    else
        return true;
}

void listOpen(List listaFicheros) // open
{
    Pos p;
    char modo[COMMAND_LENGTH] = ""; // inicializamos modo

    if (listaFicheros == NULL)
        return;
    else
    {
        for (p = listaFicheros; p != NULL; p = p->next) // para cada elemento de la lista miramos su modo para el printf
        {
            if (p->data.mode == O_CREAT)
                strcpy(modo, "O_CREAT");
            if (p->data.mode == O_EXCL)
                strcpy(modo, "O_EXCL");
            if (p->data.mode == O_RDONLY)
                strcpy(modo, "O_RDONLY");
            if (p->data.mode == O_WRONLY)
                strcpy(modo, "O_WRONLY");
            if (p->data.mode == O_RDWR)
                strcpy(modo, "O_RDWR");
            if (p->data.mode == O_APPEND)
                strcpy(modo, "O_APPEND");
            if (p->data.mode == O_TRUNC)
                strcpy(modo, "O_TRUNC");

            printf("descriptor: %d -> %s %s\n", p->data.fileDescriptor, p->data.name, modo);
        }
    }
}

void openFile(char *trozo[], List *listaFicheros) // open file m1 m2...
{
    int df, mode = 0;
    char *tr[COMMAND_LENGTH];
    Item fichero;

    trocearCadena(*trozo, tr); // troceamos el comando y lo guardamos en tr[]

    if (tr[2] == NULL && tr[1] != NULL)
    {
        printf("Modo no válido\n");
        return;
    }

    if (!strcmp(tr[2], "cr")) // asignamos a mode el permiso requerido
        mode |= O_CREAT;
    else if (!strcmp(tr[2], "ex"))
        mode |= O_EXCL;
    else if (!strcmp(tr[2], "ro"))
        mode |= O_RDONLY;
    else if (!strcmp(tr[2], "wo"))
        mode |= O_WRONLY;
    else if (!strcmp(tr[2], "rw"))
        mode |= O_RDWR;
    else if (!strcmp(tr[2], "ap"))
        mode |= O_APPEND;
    else if (!strcmp(tr[2], "tr"))
        mode |= O_TRUNC;

    if ((df = open(tr[1], mode, 0777)) == -1) // abrimos el fichero y guardamos su descriptor de archivo en df
        perror("Imposible abrir fichero");    // si devuelve -1 entonces no se pudo abrir y mostramos mensaje de error
    else
    {
        fichero.fileDescriptor = df; // en el caso de que el fichero se pudo abrir, guardamos sus datos en un registro tipo Item y lo insertamos
        strcpy(fichero.name, tr[1]);
        fichero.mode = mode;

        insertItem(fichero, listaFicheros);

        printf("Anadida entrada %d a la lista de ficheros abiertos\n", df); // dado que df también nos sirve como referencia a la posición de la lista, lo utilizamos en el printf
    }
}

bool closeDf(List *listaFicheros, int df)
{
    Pos p;

    p = findFileDescriptor(df, *listaFicheros);

    if (close(p->data.fileDescriptor) == (-1)) // close devuelve -1 si da error, y 0 si es exitoso, findFileDescriptor nos devuelve el descriptor de fichero de la posición que buscamos
    {
        perror("Imposible cerrar descriptor");
        free(p); // liberamos p, ya que en el  caso de que el fileDescriptor sea -1 significa que hemos creado un tipo Pos temporal que debemos liberar una vez acabado su uso
        return false;
    }
    else // si no da error, entonces eliminamos el fichero de la lista de ficheros abiertos
    {
        deleteAtPosition(p, listaFicheros);
        return true;
    }
}

