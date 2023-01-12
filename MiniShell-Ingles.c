#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <parser.h>

// Structure to store the jobs
typedef struct
{
    char instruction[1024]; // String of the instruction
    int size;               // Number of commands
    pid_t pids[50];         // array of pids
    int check[50];          // array that checks that pid has finished
} jobs;

jobs killCommand; // Global variable that will be used when the user wants to fetch a command from the fg and execute the sign signal.

// This function checks that the number entered for the umask is valid.
bool checkOctal(char *n)
{
    int i, aux;   // Variable for the for, Variable for converting char to int
    // Here we check that it has 4 digits or less
    if (strlen(n) > 4)
    {
        return false;
    }
    // In the for we check that all its digits are less than 8
    for (i = strlen(n) - 1; i >= 0; i--)
    {
        aux = n[i] - 48;
        if ((aux < 0) || (aux > 7))
        {
            return false;
        }
    }
    return true;
}

// Print a custom prompt.
void prompt(char *us, char *wd, char *hostname)
{
    printf("\033[0;32m"); // green
    printf("%s@%s", us, hostname);
    printf("\033[0m"); // white
    printf(":");
    printf("\033[0;34m"); // blue
    printf("%s", wd);
    printf("\033[0m"); // white
    printf("$ ");
}

// Redirect standard input and output as requested in the instruction.
void redirect(char *in, char *ou, char *err, bool c1)
{
    FILE *fi, *fo, *fe; // Variables to store the FILE of the standard input, standard output and standard error.
    if (in != NULL)     // Checks if the input has been requested to be redirected
    {
        fi = fopen(in, "r");
        if (fi != NULL) // If the redirects address exists
        {
            dup2(fileno(fi), STDIN_FILENO);
        }
        else // If it does not exist, an error is raised
        {
            fprintf(stderr, "%s: file or directory does not exist", in);
            exit(1);
        }
        fclose(fi);
    }
    if (c1 && ou != NULL) // Check if the standard output has been requested to be redirected.
    {
        fo = fopen(ou, "w");
        dup2(fileno(fo), STDOUT_FILENO);
        fclose(fo);
    }
    if (c1 && err != NULL) // Check if the error output has been requested to be redirected.
    {
        fe = fopen(err, "w");
        dup2(fileno(fe), STDERR_FILENO);
        fclose(fe);
    }
}

// Used to reprogram ctrl + c if nothing is running in fg.
void crlc()
{
    char wd[1024], us[1024], hostname[1024]; // Prompt variables
    getcwd(wd, sizeof(wd));                  // Initialize prompt variables
    gethostname(hostname, sizeof(hostname));
    getlogin_r(us, sizeof(us));
    printf("\n");
    prompt(us, wd, hostname);
    fflush(stdout);
}
// Used to reprogram ctrl + c if something is running in fg.
void crlc2()
{
    printf("\n");
}

// Used to reschedule ctrl + c if you just fetched something from bg to fg.
void crlc3()
{
    int i;
    for (i = 0; i < killCommand.size; i++) // kills all pid of the command.
    {
        kill(killCommand.pids[i], 9);
    }
    printf("\n");
}

// Executes n commands, receives the commands to execute, the status of the jobs and the number of commands in bg
void executeNComands(tline *line, jobs **lljobs, int num)
{
    // Statements
    jobs *ljobs;             // Will be used to store the ed of processes in bg
    ljobs = (jobs *)*lljobs; // We access the memory of the ed for the processes in bg
    pid_t pid;               // Variable to store the pid of the children
    int evenpipe[2], oddpipe[2];  // Pipes that we are going to use to communicate to children
    int j = 0;               // Variable to know if that child is even or odd
    pid_t *pidAux = malloc(line->ncommands);
    // Execution
    if (line->background == 1)
    {
        signal(SIGINT, SIG_IGN);
    }
    else
    {
        signal(SIGINT, crlc2);
    }
    if (line->ncommands > 1) // If there is more than one command we initialize the first pipe.
    {
        pipe(evenpipe);
    }
    // signal(SIGINT, crlc2); // We reprogram the signal ctrl + c to do a print(\n).
    pid = fork(); // Create the child
    if (pid == 0) // If it is the child we execute
    {
        redirect(line->redirect_input, line->redirect_output, line->redirect_error, j == (line->ncommands - 1)); // Redirect standard input and output if necessary
        if (line->ncommands > 1)                                                                                 // If there is more than one command
        {
            close(evenpipe[0]);
            dup2(evenpipe[1], STDOUT_FILENO); // Change the file descriptor of the standard output to pipe
            close(peer[1]);
        }
        execvp(line->commands[0].argv[0], line->commands[0].argv);           // We execute the command.
        fprintf(stderr, "%s: Command not found", line->commands[j].argv[0]); // If we get here it means there has been an error, we launch it and exit
        exit(1);
    }
    else
    {
        if (line->background == 1) // Checks if the processes have been requested to be executed in bg
        {
            signal(SIGINT, SIG_IGN);           // Ignore the signal ctrl + c
            ljobs[num].size = line->ncommands; // Save the number of commands in our ed of processes in bg
            ljobs[num].pids[0] = pid;          // Store the pid of the first child
        }
        else // If it has not been requested to be done in background, we save in a local ed.
        {
            pidAux[j] = pid;
        }
        if (line->ncommands > 1) // If there is more than one command we close the pipe that was open and will not be used.
        {
            close(evenpipe[1]);
        }
        for (j = 1; j < line->ncommands; j++) // For all commands that remained to be executed.
        {
            if (j % 2 == 0) // If the command number is even, the evenpipe pipe will be opened to write in it and read from oddpipe
            {
                pipe(evenpipe);
            }
            else // If the command number is odd the pipe oddpipe will be opened to write to it and read from the pipe evenpipe
            {
                pipe(oddpipe);
            }
            pid = fork(); // Create a child
            if (pid == 0) // Makes the child execute it
            {
                redirect(NULL, line->redirect_output, line->redirect_error, j == (line->ncommands - 1)); // Redirect input and output if necessary
                if (j % 2 == 0)                                                                          // If the command number is even
                {
                    dup2(oddpipe[0], STDIN_FILENO); // Change the file descriptor of the standard input to the one that was in the pipe oddpipe
                    if (j < line->ncommands - 1)   // If the command is not the last one
                    {
                        dup2(evenpipe[1], STDOUT_FILENO); // Change the file descriptor of the standard output and store it in evenpipe
                    }
                }
                else // If the command number is odd we do the same as if it is even but read from evenpipe and write to oddpipe.
                {
                    dup2(evenpipe[0], STDIN_FILENO);
                    if (j < line->ncommands - 1)
                    {
                        dup2(oddpipe[1], STDOUT_FILENO);
                    }
                }
                close(evenpipe[0]); // We close all pipes so that nothing is left open.
                close(oddpipe[1]);
                close(oddpipe[0]);
                close(evenpipe[1]);
                execvp(line->commands[j].argv[0], line->commands[j].argv);           // Execute the corresponding command
                fprintf(stderr, "%s: Command not found", line->commands[j].argv[0]); // If we have arrived here is that something has gone wrong, we have to throw an error
                exit(1);
            }
            else
            {
                if (line->background == 1) // Check if processes have been requested to run in bg
                {
                    signal(SIGINT, SIG_IGN);           // Ignore the signal ctrl + c
                    ljobs[num].size = line->ncommands; // Save the number of commands in our ed of processes in bg
                    ljobs[num].pids[0] = pid;          // Store the pid of the first child
                }
                else // If it has not been requested to be done in background, we save in a local ed.
                {
                    pidAux[j] = pid;
                }
                if (line->ncommands > 1) // If there is more than one command we close the pipe that was open and will not be used.
                {
                    close(evenpipe[1]);
                }
                for (j = 1; j < line->ncommands; j++) // For all commands that remained to be executed.
                {
                    if (j % 2 == 0) // If the command number is even, the evenpipe pipe will be opened to write in it and read from oddpipe
                    {
                        pipe(evenpipe);
                    }
                    else // If the command number is odd the pipe oddpipe will be opened to write to it and read from the pipe evenpipe
                    {
                        pipe(oddpipe);
                    }
                    pid = fork(); // Create a child
                    if (pid == 0) // Makes the child execute it
                    {
                        redirect(NULL, line->redirect_output, line->redirect_error, j == (line->ncommands - 1)); // Redirect input and output if necessary
                        if (j % 2 == 0)                                                                          // If the command number is even
                        {
                            dup2(oddpipe[0], STDIN_FILENO); // Change the file descriptor of the standard input to the one that was in the pipe oddpipe
                            if (j < line->ncommands - 1)   // If the command is not the last one
                            {
                                dup2(evenpipe[1], STDOUT_FILENO); // Change the file descriptor of the standard output and store it in evenpipe
                            }
                        }
                        else // If the command number is odd we do the same as if it is even but read from evenpipe and write to oddpipe.
                        {
                            dup2(evenpipe[0], STDIN_FILENO);
                            if (j < line->ncommands - 1)
                            {
                                dup2(oddpipe[1], STDOUT_FILENO);
                            }
                        }
                        close(evenpipe[0]); // We close all pipes so that nothing is left open.
                        close(oddpipe[1]);
                        close(oddpipe[0]);
                        close(evenpipe[1]);
                        execvp(line->commands[j].argv[0], line->commands[j].argv);           // Execute the corresponding command
                        fprintf(stderr, "%s: Command not found", line->commands[j].argv[0]); // If we have arrived here is that something has gone wrong, we have to throw an error
                        exit(1);
                    }
                    else // This code is executed by the parent
                    {
                        if (j % 2 == 0) // If the number of the last child process is even
                        {
                            dup2(oddpipe[0], evenpipe[1]); // Point the standard input of the next child process (evenpipe) to the output of the child process we just created (oddpipe)
                            close(oddpipe[0]);         // Close the pipes we don't need anymore
                            close(evenpipe[1]);
                            if (j == (line->ncommands - 1)) // If it is the last command
                            {
                                close(evenpipe[0]); // We close this pipe since no process will have to read the output of this one.
                            }
                        }
                        else // If the number of process is odd we do the same but exchanging evenpipe and oddpipe.
                        {
                            dup2(odd[0], odd[1]);
                            close(oddpair[0]);
                            close(odd - even[1]);
                            if (j == (line->ncommands - 1))
                            {
                                close(oddpipe[0]);
                            }
                        }
                        if (line->background == 1) // If bg is to be executed in bg.
                        {
                            ljobs[num].pids[j] = pid; // We store the pid of the child in an ed
                        }
                        else // If to execute in fg
                        {
                            pidAux[j] = pid; // We store the child's pid in an auxiliary edit
                        }
                    }
                }

                if (line->background == 0) // If we execute in fg.
                {
                    for (j = 0; j < line->ncommands; j++) // For each process saved in auxiliary ed
                    {
                        waitpid(pidAux[j], NULL, 0); // We wait it until it finishes
                    }
                }
            }
            free(pidAux);
            signal(SIGINT, crlc); // We reprogram the signal ctrl + c and leave it as it was before the function.
        }

        // Pass a command to foreground
        void fgCommand(tcommand * com, jobs ljobs[], int number)
        {
            signal(SIGINT, crlc3); // select a handler when crtl+c is done
            int i, j;              // start varibles for the for
            if (com->argc == 1)    // in case there is no argument we do the following.
            {
                killCommand = ljobs[number - 1];           // we set the global variable in case the user wants to kill the process he just sent to fg
                for (i = 0; i < ljobs[number - 1].size; i++) // we wait for all parts of last instuction
                {
                    waitpid(ljobs[number - 1].pids[i], NULL, 0);
                }
            }
            else
            {
                killCommand = ljobs[atoi(com->argv[1])];           // We set the global variable in case the user wants to kill the process he just sent to fg
                for (i = 0; i < ljobs[atoi(com->argv[1])].size; i++) // we wait for all the parts of the selected instuction
                {
                    waitpid(ljobs[atoi(com->argv[1])].pids[i], NULL, 0);
                }
                for (j = i; j < number; j++) // let's update the list of instructions
                {
                    ljobs[j] = ljobs[j + 1];
                }
            }
            signal(SIGINT, crlc);
        }

        // Change the working directory
        void cdCommand(tcommand * com)
        {
            if (com->argc == 1) // If no parameters are received change it to /home/user
            {
                chdir(getenv("HOME"));
            }
            else if (com->argc == 2)
            {
                if (chdir(com->argv[1]) != 0) // If the directory is incorrect an error is raised
                {
                    fprintf(stderr, "cd: %s is not a directory", com->argv[1]);
                }
            }
            else // If more than one parameter is received an error is raised.
            {
                fprintf(stderr, "cd: too many arguments");
            }
        }

        // Execute the umask command
        void umaskCommand(tcommand * com, mode_t * mask)
        {
            int maskAux = *mask;
            int number = 4;
            int octal;
            if (com->argc == 1) // If passed without parameters we show the current mask
            {
                if (maskAux == 0)
                {
                    numero - ;
                }
                while (mascaraAux > 0) // We calculate the number of zeros to print.
                {
                    mascaraAux = mascaraAux / 10;
                    numero--;
                }
                while (numero > 0) // Print the required number of zeros.
                {
                    printf("0");
                    numero--;
                }
                printf("%i", *mask);
            }
            else // If it receives any parameter we change the mask.
            {
                if (checkOctal(com->argv[1])) // If the parameter is a correct octal we change the mask
                {
                    *mask = atoi(com->argv[1]);
                    sscanf(com->argv[1], "%o", &octal); // We calculate the octal of the number we have been given
                    umask(octal);                       // Change the mask
                }
                else // If the parameter is not a correct octal we return an error.
                {
                    fprintf(stderr, "%s: invalid must be a 4-digit octal number or less", com->argv[1]);
                    return;
                }
            }
        }

        // Command to close the program
        void exitCommand(int number, jobs ljobs[])
        {
            int i, j;                    // Variables for for
            for (j = 0; j < number; j++) // Close all processes in bg
            {
                for (i = 0; i < ljobs[j].size; i++)
                {
                    kill(ljobs[j].pids[i], 9);
                }
            }
            free(ljobs); // We free the background process ed.
            exit(0);     // We exit the minishell
        }

        // Function to show processes in bg
        void showjobs(jobs ljobs[], int *number, int control)
        {
            int i, j, p, cont; // variables of the for, index of the changes and counter
            int change[50];    // array of finished pids
            p = 0;
            for (i = 0; i < (*number); i++) // check all instructions in bg
            {
                cont = 0;
                for (j = 0; j < ljobs[i].size; j++) // check all parts of the instructions
                {
                    if ((waitpid(ljobs[i].pids[j], NULL, WNOHANG) == ljobs[i].pids[j]) || (ljobs[i].check[j])) // check if a pid has finished
                    {
                        cont++;
                        ljobs[i].check[j] = 1;
                    }
                    else
                    {
                        if (control) // control is a variable that will be activated in the jobs but not when checking after an instruction if something is finished
                        {
                            printf("[%d] Executing %s", i, ljobs[i].instruction);
                        }
                        break;
                    }
                }
                if (cont == ljobs[i].size)
                {
                    printf("[%d] Done %s", i, ljobs[i].instruction);
                    change[p] = i;
                    p++;
                }
            }
            if (p > 0)
            {
                while (p > 0)
                {
                    for (j = change[0]; j <= (*number); j++)
                    {
                        ljobs[j] = ljobs[j + 1];
                    }
                    *number = *number - 1;
                    p--;
                }
            }
        }
        int main(void)
        {
            // Declarations of the variables that we are going to need
            char buffer[1024];                       // Buffer in which we are going to store the standard input
            char wd[1024], us[1024], hostname[1024]; // Store variables to display the prompt
            tline *line;                             // Let's store the tokenized commands in this variable
            jobs *ljobs = malloc(sizeof(jobs) * 50); // ED to store the processes in bg
            int number = 0;                          // Number of process in bg
            mode_t mask = 22;                        // Current mask
            umask(18);                               // We set the mask to the linux default mask

            // Program logic
            signal(SIGINT, crlc); // Reprogram the signal ctrl + c

            getcwd(wd, sizeof(wd));                  // Initialize prompt variables
            gethostname(hostname, sizeof(hostname)); //
            getlogin_r(us, sizeof(us));

            prompt(us, wd, hostname); // Print out the prompt

            while (fgets(buffer, 1024, stdin)) // Loop to read the standard input
            {
                line = tokenize(buffer);                  // Tokenize standard input
                if (line != NULL && line->ncommands != 0) // Let's check that the line is correct and also that it's not just an intro
                {
                    if (line->ncommands == 1) // Check that a command has been entered
                    {
                        if (strcmp(line->commands[0].argv[0], "cd") == 0) // We enter by this if if the command is a cd
                        {
                            cdCommand(line->commands);
                            getcwd(wd, sizeof(wd));
                        }
                        else if (strcmp(line->commands[0].argv[0], "umask") == 0) // We enter by this if if the command is an umask.
                        {
                            umaskCommand(line->commands, &mask);
                        }
                        else if (strcmp(line->commands[0].argv[0], "jobs") == 0) // We enter by this if the command is a jobs.
                        {
                            showjobs(ljobs, &number, 1);
                        }
                        else if (strcmp(line->commands[0].argv[0], "fg") == 0) // We enter by this if the command is a fg.
                        {
                            if (number > 0)
                            {
                                if (line->commands->argc > 1)
                                {
                                    if (atoi(line->commands->argv[1]) <= number - 1)
                                    {
                                        fgCommand(line->commands, ljobs, numero);
                                        numero - ;
                                    }
                                    else
                                    {
                                        printf("fg: no such job exists");
                                    }
                                }
                                else
                                {
                                    fgCommand(line->commands, ljobs, number);
                                    numero - ;
                                }
                            }
                            else
                            {
                                printf("fg: no such job exists");
                            }
                        }
                        else if (strcmp(line->commands[0].argv[0], "exit") == 0) // We enter this one if the command is a job.
                        {
                            exitCommand(number, ljobs);
                        }
                        else // If it doesn't match any of the above we execute the external shell commands.
                        {
                            executeNComands(line, &ljobs, numero);
                            if (line->background == 1)
                            {

                                printf("[%d] %d", number, ljobs[number].pids[0]);
                                strcpy(ljobs[number].instruction, buffer);
                                number++;
                            }
                        }
                    }
                    else // If there is more than one command, we execute the external ones from the shell.
                    {
                        executeNComands(line, &ljobs, number);
                        if (line->background == 1)
                        {
                            printf("[%d] %d", number, ljobs[number].pids[0]);
                            strcpy(ljobs[number].instruction, buffer);
                            number++;
                        }
                    }
                }
                showjobs(ljobs, &number, 0); // Show if there are any processes in bg that have terminated.
                prompt(us, wd, hostname);    // Show the prompt
            }
        }