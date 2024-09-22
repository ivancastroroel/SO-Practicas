//INCLUDE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/utsname.h>
#include<stdbool.h>

//DEFINE
#define MAX_INPUT 1024
#define MAX_TROZOS 10


//Declaración de funciones
void imprimirPrompt();
void leerEntrada(char *entrada);
void procesarEntrada(char *entrada);
int TrocearCadena(char * entrada, char * trozos[]);

int main(){
    char entrada[MAX_INPUT];


    while (1){
        imprimirPrompt();
        leerEntrada(entrada);
        procesarEntrada(entrada);
    }

    return 0;
}

void imprimirPrompt(){
    printf("mi_shell> ");
}

void leerEntrada(char * entrada){
    fgets(entrada, MAX_INPUT, stdin);
}

void procesarEntrada(char * entrada){
    char *trozos[MAX_TROZOS];
    char buf[1024];
    TrocearCadena(entrada, trozos);
    
    //AUTHORS
    if(strcmp(trozos[0], "authors")==0){
        if (trozos[1]==NULL){
            printf("Ivan - ivan.castro.roel\n");
            printf("Lucas - l.garcia-boenter\n");
        }
        else if (strcmp(trozos[1], "-l")==0){
                printf("ivan.castro.roel\n");
                printf("l.garcia-boenter\n");
        }
        else if (strcmp(trozos[1], "-n")==0){
                printf("Ivan\n");
                printf("Lucas\n");
        }
    }
    //PID
    else if (strcmp(trozos[0], "pid")==0){
        printf("%d\n", getpid());
    }
    //PPID
    else if (strcmp(trozos[0], "ppid")==0){
        printf("%d\n", getppid());
    }
    //CD
    else if (strcmp(trozos[0], "cd")==0){
        if(strcmp(trozos[0], "NULL")==0){
            getcwd(buf, sizeof(buf));
            printf("Ruta actual:%s\n", buf);
        }
        else{
            //chdir();
        }
    }
    else{
        printf("\nComando no reconocido\n");
    }
}

int TrocearCadena(char * cadena, char * trozos[])
{ 
    int i=1;

    if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
        return 0;

    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
        i++;

    return i; 
}
