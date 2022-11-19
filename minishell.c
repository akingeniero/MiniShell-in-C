#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"

void prompt(char *us, char *wd, char *hostname)
{
    printf("\033[0;32m");
    printf("%s@%s", us, hostname);
    printf("\033[0m");
    printf(":");
    printf("\033[0;34m");
    printf("%s", wd);
    printf("\033[0m");
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
    if (c1 == true && ou != NULL)
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

int main(void)
{
    char buffer[1024];
    char wd[1024], us[1024], hostname[1024];
    tline *line;
    pid_t pid;
    int j = 0;
    int p[2], p1[2];
    getcwd(wd, sizeof(wd));
    gethostname(hostname, sizeof(hostname));
    getlogin_r(us, sizeof(us));
    prompt(us, wd, hostname);
    while (fgets(buffer, 1024, stdin))
    {
        line = tokenize(buffer);
        if (line->ncommands != 0)
        {
            pipe(p);
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
                close(p[1]);
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
                            if (j > 0)
                            {
                                dup2(p1[0], STDIN_FILENO);
                            }
                            if (j < line->ncommands - 1)
                            {
                                dup2(p[1], STDOUT_FILENO);
                            }
                        }
                        else
                        {
                            if (j > 0)
                            {
                                dup2(p[0], STDIN_FILENO);
                            }
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
                    }
                }
                for (j = 0; j < line->ncommands; j++)
                {
                    wait(NULL);
                }
            }
        }
        prompt(us, wd, hostname);
    }
}