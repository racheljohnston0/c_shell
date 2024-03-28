#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_PATH_SIZE 200
#define MAX_PATH_COUNT 100
#define MAX_ARGUMENTS 10

int num_of_paths = 0;

void printError()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void changeDirectory(char *dir)
{
    int cd_flag = chdir(dir);
    if(cd_flag != 0)
    {
        printError();
    }
}

void redirect(char (*path)[MAX_PATH_SIZE], char *args[], int arg_count)
{
    if (strchr(args[1], '>') == strrchr(args[1], '>') && (arg_count <= 3)) {

        //need to tokenize the second part of the command
        char line[100];
        char *tokens[MAX_ARGUMENTS];
        strcpy(line, args[1]);
        line[strcspn(line, "\n")] = '\0';
        int count = 0;
        char *token = strtok(line, ">");

        while (token != NULL && count < MAX_ARGUMENTS - 1) {
            tokens[count] = token;
            count++;
            token = strtok(NULL, " ");
        }

        if (count != 0) {
            char *temp_path = malloc(sizeof(path[0]));
            strcpy(temp_path, path[0]);
            strcat(temp_path, "/");
            strcat(temp_path, args[0]);

            strcpy(args[1], tokens[0]);

            pid_t pid = fork();
            if (pid < 0) {
                printError();
                exit(1);
            } else if (pid == 0) { //child process executes command
                //close standard output
                int fd = open(tokens[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
                if (fd == -1) {
                    printError();
                }
                //redirect output to the new file
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    exit(EXIT_FAILURE);
                }
                execv(temp_path, args);
                close(fd);
            } else { //parent process waits for child to complete
                int status;
                waitpid(pid, &status, 0);
            }
        } else {
            //no arguments after '>'
            printError();
        }
    } else {
        //Too many '>' or too many arguments
        printError();
    }
} // end redirect function

void executeParallelCmd() {

} // end executeParallelCmd()

int executeAllOtherCommands(char (*path)[MAX_PATH_SIZE], char *args[], int i, int arg_count)
{
    char *temp_path = malloc(sizeof(path[i]));
    strcpy(temp_path, path[i]);
    strcat(temp_path, "/");
    strcat(temp_path, args[0]);

    int redirection_flag = -1;
    int parallel_cmd_flag = -1;

    if (args[i + 1] != NULL) {
        if (strchr(args[i + 1], '>') != NULL)
        {
            redirection_flag = 0;
        }
        else if (strchr(args[i + 1], '&') != NULL)
        {
            parallel_cmd_flag = 0;
        }
    }
    if (redirection_flag == 0) {
        redirect(path, args, arg_count);
    } else if (parallel_cmd_flag == 0) {
        executeParallelCmd();
    }
    else if (access(temp_path, X_OK) == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            printError();
            exit(1);
        } else if (pid == 0) { //child process executes command
            execv(temp_path, args);
        } else { //parent process waits for child to complete
            int status;
            waitpid(pid, &status, 0);
        }
    } else { //file not found
        return -1;
    }
    free(temp_path);
    return 0;
}

void executeCommands(char *args[], int arg_count, char (*path)[200], bool parallel_command_flag) {
    if (parallel_command_flag == true){
        if (arg_count > 1) {
            printf("Ready to receive parallel commands\n");
        }
    }
    else if (strcmp(args[0], "exit") == 0) {
        if (arg_count > 1) {
            printError();
        } else {
            exit(0);
        }
    } else if (strcmp(args[0], "cd") == 0) {
       if ((arg_count == 1) || (arg_count > 2)) {
            printError();
        } else if (args[1] != NULL) {
            changeDirectory(args[1]);
        }
    } else if (strcmp(args[0], "path") == 0) {
        num_of_paths = 0;
        //empty paths
        for (int i = 0; i < MAX_PATH_COUNT; i++) {
            for (int j = 0; j < MAX_PATH_SIZE; j++) {
                path[i][j] = '\0';
            }
        }
        if (args[1] != NULL) {
            for (int i = 1; i < arg_count; i++) {
                char cwd[128];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    strcat(cwd, "/");
                    strcat(cwd, args[i]);
                    strcpy(path[num_of_paths], cwd);
                    num_of_paths++;
                }
            }
        }
    } else { // all other commands
        //make path go back to bin at some point
        int num_of_failures = 0;
        if (num_of_paths == 0) {
            if (executeAllOtherCommands(path, args, 0, arg_count) == -1) {
                printError();
            }
        } else {
            for (int i = 0; i < num_of_paths; i++) {
                if (executeAllOtherCommands(path, args, i, arg_count) == -1) {
                    num_of_failures++;
                }
            }
        }
        if ((num_of_failures == num_of_paths) && (num_of_paths > 0)) {
            printError();
        }
    }
} // end function