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
    int sto = 0, sti = 0, ste = 0;
    FILE *fi, *fo, *fe;
    pid_t pid;
    int p[2];
    printf("msh> ");
    while (fgets(buffer, 1024, stdin))
    {
        line = tokenize(buffer);
        if (line->redirect_input != NULL)
        {
            dup2(STDIN_FILENO, sti);
            fi = fopen(line->redirect_input, "r");
            dup2(fileno(fi), STDIN_FILENO);
        }
        if (line->redirect_output != NULL)
        {
            
            dup2(STDOUT_FILENO, sto);
            fo = fopen(line->redirect_output, "w");
            dup2(fileno(fo), STDOUT_FILENO);
        }
        if (line->redirect_error != NULL)
        {
            dup2(STDERR_FILENO, ste);
            fe = fopen(line->redirect_error, "w");
            dup2(fileno(fe), STDERR_FILENO);
        }
        pid = fork();
        if (pid == 0)
        {
            execvp(line->commands[0].argv[0], line->commands[0].argv);
            fprintf(stderr, "%s: No se encuentra el mandato\n", line->commands[0].argv[0]);
            exit(1);
        }
        else
        {

            wait(NULL);
        }
        if (line->redirect_input != NULL)
        {
            dup2(sti, STDIN_FILENO);
            fclose(fi);
        }
        if (line->redirect_output != NULL)
        {
            dup2(sto, STDOUT_FILENO);
            fclose(fo);
        }
        if (line->redirect_error != NULL)
        {
            dup2(ste, STDERR_FILENO);
            fclose(fe);
        }
        printf("msh> ");
    }
}