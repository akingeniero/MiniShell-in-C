#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "parser.h"

int main(void)
{
    char buffer[1024];
    tline *line;
    FILE *fi, *fo, *fe;
    pid_t pid;
    int j = 0;
    int p[2];
    printf("msh> ");
    while (fgets(buffer, 1024, stdin))
    {
        line = tokenize(buffer);
        if (line == NULL)
        {
            continue;
        }
        pipe(p);
        pid = fork();
        if (pid == 0)
        {
            if (line->redirect_input != NULL)
            {
                fi = fopen(line->redirect_input, "r");
                dup2(fileno(fi), STDIN_FILENO);
                fclose(fi);
            }
            if (j == (line->ncommands - 1) && line->redirect_output != NULL)
            {
                fo = fopen(line->redirect_output, "w");
                dup2(fileno(fo), STDOUT_FILENO);
                fclose(fo);
            }
            if (j == (line->ncommands - 1) && line->redirect_error != NULL)
            {
                fe = fopen(line->redirect_error, "w");
                dup2(fileno(fe), STDERR_FILENO);
                fclose(fe);
            }
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
            j++;
            pid = fork();
            if (pid == 0)
            {
                if (j == (line->ncommands - 1) && line->redirect_output != NULL)
                {
                    fo = fopen(line->redirect_output, "w");
                    dup2(fileno(fo), STDOUT_FILENO);
                    fclose(fo);
                }
                if (j == (line->ncommands - 1) && line->redirect_error != NULL)
                {
                    fe = fopen(line->redirect_error, "w");
                    dup2(fileno(fe), STDERR_FILENO);
                    fclose(fe);
                }
                if (j > 0)
                {
                    dup2(p[0], STDIN_FILENO);
                    close(p[0]);
                }
                execvp(line->commands[1].argv[0], line->commands[1].argv);
                fprintf(stderr, "%s: No se encuentra el mandato\n", line->commands[j].argv[0]);
                exit(1);
            }
            else
            {
                close(p[0]);
                for (j = 0; j < line->ncommands; j++)
                {
                    wait(NULL);
                }
                printf("msh> ");
            }
        }
    }
}