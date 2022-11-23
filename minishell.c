#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"

typedef struct
{
    pid_t pid;
    char *instruccion;
    int curso;
    int tamaño;
    pid_t otros[50];
    int otros2[50];
} jobs;

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

void redirect(char *in, char *ou, char *err, bool c1)
{
    FILE *fi, *fo, *fe;
    if (in != NULL)
    {
        fi = fopen(in, "r");
        dup2(fileno(fi), STDIN_FILENO);
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

void crlc(int sig)
{
    char wd[1024], us[1024], hostname[1024];
    getcwd(wd, sizeof(wd));
    gethostname(hostname, sizeof(hostname));
    getlogin_r(us, sizeof(us));
    printf("\n");
    prompt(us, wd, hostname);
    fflush(stdout);
}

void crlc2(int sig)
{
    printf("\n");
}

void executeNComands(tline *line, jobs ljobs[], int num)
{
    pid_t pid;
    int p[2], p1[2];
    int j = 0;
    if (line->ncommands > 1)
    {
        pipe(p);
    }
    signal(SIGINT, crlc2);
    pid = fork();
    if (line->background == 1)
    {
        signal(SIGINT, SIG_IGN);
        ljobs[num].tamaño = line->ncommands;
    }
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
            }
        }
        if (line->background == 0)
        {
            for (j = 0; j < line->ncommands; j++)
            {
                wait(NULL);
            }
        }
        else
        {
            ljobs[num].instruccion = &(line->commands[0].argv);
            ljobs[num].pid = pid;
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
    else
    {
        chdir(com->argv[1]);
    }
}

void umaskCommand(tcommand *com)
{
    mode_t masc = com->argv[1];
    umask(masc);
}

void mostrarjobs(jobs ljobs[], int numero)
{
    printf("jobs %d \n", numero);
    int i, j, cont;
    for (i = 0; i <= numero; i++)
    {
        if ((ljobs[i].tamaño > 1))
        {
            if (!ljobs[i].curso)
            {
                for (j = 1; j <= ljobs[j].tamaño + 1; j++)
                {
                    if ((waitpid(ljobs[i].otros[j], NULL, WNOHANG) == ljobs[i].otros[j]) || (ljobs[i].otros2[j]))
                    {
                        cont++;
                        ljobs[i].otros2[j] = 1;
                    }
                    else
                    {
                        printf("[%d]  bakground        %d \n", i, ljobs[i].pid);
                        break;
                    }
                }
                if (cont == ljobs[i].tamaño)
                {
                    printf("[%d]  murio        %d \n", i, ljobs[i].pid);
                    ljobs[i].curso = 1;
                }
            }
        }
        else
        {
            printf("1");
            if ((waitpid(ljobs[i].pid, NULL, WNOHANG) == ljobs[i].pid) || ljobs[i].curso)
            {
                printf("[%d]  murio    %d \n", i, ljobs[i].pid);
                ljobs[i].curso = 1;
            }
            else
            {
                printf("[%d]  bakground        %d \n", i, ljobs[i].pid);
            }
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
    jobs ljobs[50];
    int numero = 0;
    // Lógica de programa
    signal(SIGINT, crlc);
    prompt(us, wd, hostname);
    while (fgets(buffer, 1024, stdin))
    {
        line = tokenize(buffer);
        if (line->ncommands != 0)
        {
            if (line->ncommands == 1)
            {
                if (strcmp(line->commands[0].argv[0], "cd") == 0)
                {

                    cdCommand(line->commands);
                    getcwd(wd, sizeof(wd));
                }
                else if (strcmp(line->commands[0].argv[0], "exit") == 0)
                {
                    exit(0);
                }
                else if (strcmp(line->commands[0].argv[0], "umask") == 0)
                {
                    umaskCommand(line->commands);
                }
                else if (strcmp(line->commands[0].argv[0], "jobs") == 0)
                {
                    mostrarjobs(ljobs, numero - 1);
                }
                else
                {
                    executeNComands(line, &ljobs, numero);
                    if (ljobs[numero].pid != NULL)
                    {
                        numero++;
                    }
                }
            }
            else
            {
                executeNComands(line, &ljobs, numero);
                if (ljobs[numero].pid != NULL)
                {
                    numero++;
                }
            }
        }
        prompt(us, wd, hostname);
    }
}