#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include <sys/stat.h>

// Estructura para guardar el jobs
typedef struct
{
    char instruccion[1024];
    int tamaño;
    pid_t otros[50];
    int otros2[50];
} jobs;

int mascara = 0002;

// Imprime un prompt personalizado
void prompt(char *us, char *wd, char *hostname)
{
    printf("\033[0;32m"); // verde
    printf("%s@%s", us, hostname);
    printf("\033[0m"); // blanco
    printf(":");
    printf("\033[0;34m"); // azul
    printf("%s", wd);
    printf("\033[0m"); // blanco
    printf("$ ");
}

// Redirecciona la entrada y salida estándar según se pida en la instrucción
void redirect(char *in, char *ou, char *err, bool c1)
{
    FILE *fi, *fo, *fe;
    if (in != NULL)
    {
        fi = fopen(in, "r");
        if (fi != NULL){
                    dup2(fileno(fi), STDIN_FILENO);
        }else{
            fprintf(stderr,"%s: no existe el archivo o el directorio\n", in);
            exit(1);
        }
        fclose(fi);
    }
    if (c1 && ou != NULL)
    {
        fo = fopen(ou, "w");
        dup2(fileno(fo), STDOUT_FILENO);
        fclose(fo);
    }
    if (c1 && err != NULL)
    {
        fe = fopen(err, "w");
        dup2(fileno(fe), STDERR_FILENO);
        fclose(fe);
    }
}

// Sirven para reprogramar control + C
void crlc()
{
    char wd[1024], us[1024], hostname[1024];
    getcwd(wd, sizeof(wd));
    gethostname(hostname, sizeof(hostname));
    getlogin_r(us, sizeof(us));
    printf("\n");
    prompt(us, wd, hostname);
    fflush(stdout);
}

void crlc2()
{
    printf("\n");
}

void executeNComands(tline *line, jobs **lljobs, int num)
{
    jobs *ljobs;
    ljobs = (jobs *)*lljobs;
    pid_t pid;
    int p[2], p1[2];
    int j = 0;
    pid_t *pru = malloc(line->ncommands);
    if (line->ncommands > 1)
    {
        pipe(p);
    }
    signal(SIGINT, crlc2);
    pid = fork();
    if (pid == 0)
    {
        redirect(line->redirect_input, line->redirect_output, line->redirect_error, j == (line->ncommands - 1));
        if (line->ncommands > 1)
        {
            close(p[0]);
            dup2(p[1], STDOUT_FILENO);
            close(p[1]);
        }
        execvp(line->commands[0].argv[0], line->commands[0].argv);
        fprintf(stderr, "%s: No se encuentra el mandato\n", line->commands[j].argv[0]);
        exit(1);
    }
    else
    {
        if (line->background == 1)
        {
            signal(SIGINT, SIG_IGN);
            ljobs[num].tamaño = line->ncommands;
            ljobs[num].otros[0] = pid;
        }
        else
        {
            pru[j] = pid;
        }
        if (line->ncommands > 1)
        {
            close(p[1]);
        }
        for (j = 1; j < line->ncommands; j++)
        {
            if (j % 2 == 0)
            {
                pipe(p);
            }
            else
            {
                pipe(p1);
            }
            pid = fork();
            if (pid == 0)
            {
                redirect(NULL, line->redirect_output, line->redirect_error, j == (line->ncommands - 1));
                if (j % 2 == 0)
                {
                    dup2(p1[0], STDIN_FILENO);
                    if (j < line->ncommands - 1)
                    {
                        dup2(p[1], STDOUT_FILENO);
                    }
                }
                else
                {
                    dup2(p[0], STDIN_FILENO);
                    if (j < line->ncommands - 1)
                    {
                        dup2(p1[1], STDOUT_FILENO);
                    }
                }
                close(p[0]);
                close(p1[1]);
                close(p1[0]);
                close(p[1]);
                execvp(line->commands[j].argv[0], line->commands[j].argv);
                fprintf(stderr, "%s: No se encuentra el mandato\n", line->commands[j].argv[0]);
                exit(1);
            }
            else
            {
                if (j % 2 == 0)
                {
                    dup2(p1[0], p[1]);
                    close(p1[0]);
                    close(p[1]);
                    if (j == (line->ncommands - 1))
                    {
                        close(p[0]);
                    }
                }
                else
                {
                    dup2(p[0], p1[1]);
                    close(p[0]);
                    close(p1[1]);
                    if (j == (line->ncommands - 1))
                    {
                        close(p1[0]);
                    }
                }
                if (line->background == 1)
                {
                    ljobs[num].otros[j] = pid;
                }
                else
                {
                    pru[j] = pid;
                }
            }
        }

        if (line->background == 0)
        {
            for (j = 0; j < line->ncommands; j++)
            {
                waitpid(pru[j], NULL, 0);
            }
        }
    }
    signal(SIGINT, crlc);
}

void fgCommand(tcommand *com, jobs ljobs[], int numero)
{
    signal(SIGINT, crlc2);
    int i, j;
    if (com->argc == 1)
    {
        for (i = 0; i < ljobs[numero - 1].tamaño; i++)
        {
            waitpid(ljobs[numero - 1].otros[i], NULL, 0);
        }
    }
    else
    {
        for (i = 0; i < ljobs[atoi(com->argv[1])].tamaño; i++)
        {
            waitpid(ljobs[atoi(com->argv[1])].otros[i], NULL, 0);
        }
        for (j = i; j < numero; j++)
        {
            ljobs[j] = ljobs[j + 1];
        }
    }
    signal(SIGINT, crlc);
}

void cdCommand(tcommand *com)
{
    if (com->argc == 1)
    {
        chdir(getenv("HOME"));
    }
    else if(com->argc == 2)
    {
        if (chdir(com->argv[1]) != 0)
        {
           fprintf(stderr, "cd: %s no es un directorio\n", com->argv[1]);
        }
    }else{
        fprintf(stderr, "cd: demasiados argumentos\n");
    }
}

void umaskCommand(tcommand *com,int *mascara)
{
    int aux = (int) *mascara;
    if (com->argc ==1)
    {
        printf("%d \n",*mascara);
    }
    else
    {
        aux =  atoi(com->argv[1]);
        umask( aux);
    }
}

void exitCommand(int numero, jobs ljobs[])
{
    int i, j;
    for (j = 0; j < numero; j++)
    {
        for (i = 0; i < ljobs[j].tamaño; i++)
        {
            kill(ljobs[j].otros[i], 9);
        }
    }
    exit(0);
}

void mostrarjobs(jobs ljobs[], int *numero)
{
    int i, j, p, cont;
    int h[50];
    p = 0;
    for (i = 0; i < (*numero); i++)
    {
        cont = 0;
        for (j = 0; j < ljobs[i].tamaño; j++)
        {
            if ((waitpid(ljobs[i].otros[j], NULL, WNOHANG) == ljobs[i].otros[j]) || (ljobs[i].otros2[j]))
            {
                cont++;
                ljobs[i].otros2[j] = 1;
            }
            else
            {
                printf("[%d] Ejecutando        %s", i, ljobs[i].instruccion);
                break;
            }
        }
        if (cont == ljobs[i].tamaño)
        {
            printf("[%d]  Hecho        %s", i, ljobs[i].instruccion);
            h[p] = i;
            p++;
        }
    }
    if (p > 0)
    {
        while (p > 0)
        {
            for (j = h[0]; j <= (*numero); j++)
            {
                ljobs[j] = ljobs[j + 1];
            }
            *numero = *numero - 1;
            p--;
        }
    }
}

int main(void)
{
    // Declaraciones
    char buffer[1024];
    char wd[1024], us[1024], hostname[1024];
    getcwd(wd, sizeof(wd));
    gethostname(hostname, sizeof(hostname));
    getlogin_r(us, sizeof(us));
    tline *line;
    jobs *ljobs = malloc(sizeof(jobs) * 50);
    int numero = 0;

    // Lógica de programa
    signal(SIGINT, crlc);
    prompt(us, wd, hostname);
    while (fgets(buffer, 1024, stdin))
    {
        line = tokenize(buffer);
        if (line != NULL && line->ncommands != 0)
        {
            if (line->ncommands == 1)
            {
                if (strcmp(line->commands[0].argv[0], "cd") == 0)
                {
                    cdCommand(line->commands);
                    getcwd(wd, sizeof(wd));
                }
                else if (strcmp(line->commands[0].argv[0], "umask") == 0)
                {
                    umaskCommand(line->commands,&mascara);
                }
                else if (strcmp(line->commands[0].argv[0], "jobs") == 0)
                {
                    mostrarjobs(ljobs, &numero);
                }
                else if (strcmp(line->commands[0].argv[0], "fg") == 0)
                {
                    fgCommand(line->commands, ljobs, numero);
                    numero--;
                }
                else if (strcmp(line->commands[0].argv[0], "exit") == 0)
                {
                    exitCommand(numero, ljobs);
                }
                else
                {
                    executeNComands(line, &ljobs, numero);
                    if (ljobs[numero].otros[0] != 0)
                    {

                        printf("[%d] %d \n",numero,ljobs[numero].otros[0]);
                        strcpy(ljobs[numero].instruccion, buffer);
                        numero++;
                    }
                }
            }
            else
            {
                executeNComands(line, &ljobs, numero);
                if (ljobs[numero].otros[0] != 0)
                {
                    printf("[%d] %d \n",numero,ljobs[numero].otros[-1]);
                    strcpy(ljobs[numero].instruccion, buffer);
                    numero++;
                }
            }
        }
        prompt(us, wd, hostname);
    }
}