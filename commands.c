#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_PATH_SIZE 200
#define MAX_PATH_COUNT 100
#define MAX_ARGUMENTS 10
#define MAX_COMMANDS 10

int num_of_paths = 0;
bool parallel_cmd_flag = false;

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
    //printf("in redirect\n");
    if (strchr(args[1], '>') == strrchr(args[1], '>') && (arg_count <= 3)) {
        //need to tokenize the second part of the command
        //printf("passed first check\n");
        char line[100];
        char *tokens[MAX_ARGUMENTS];
        int index = 0;

        for (int i = 0; i < arg_count; i++)
        {
            strcpy(&line[index], args[i]);
            index += strlen(args[i]);
        }

        if (!parallel_cmd_flag) {
            //printf("Parallel command = false\n");
            strcpy(line, args[1]);
        }

        //strcpy(line, args[1]);
        line[strcspn(line, "\n")] = '\0';
        int count = 0;
        char *token = strtok(line, ">");

        while (token != NULL && count < MAX_ARGUMENTS - 1) {
            tokens[count] = token;
            count++;
            token = strtok(NULL, " ");
        }

        if (count != 0) {
            int paths_tried = 0;
            char *temp_path = malloc(MAX_PATH_SIZE);
            strcpy(temp_path, path[paths_tried]);
            strcat(temp_path, "/");
            strcat(temp_path, args[0]);

            //printf("Temp path: %s\n", temp_path);

            //if (!parallel_cmd_flag) {
                strcpy(args[1], tokens[0]);
           // }

            for (int i = 0; i < arg_count; i++)
            {
                //printf("args[%d]: %s\n", i, args[i]);
            }

            while ((access(temp_path, X_OK) != 0) && paths_tried < num_of_paths)
            {
                strcpy(temp_path, path[paths_tried]);
                strcat(temp_path, "/");
                strcat(temp_path, args[0]);
                //printf("temp path: %s\n", temp_path);
                paths_tried++;
            }

            //printf("about to fork() in redirect\n");
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
                close(fd);
                if (pid == 0) {
                    execv(temp_path, args);
                }
            } else { //parent process waits for child to complete
                int status;
                waitpid(pid, &status, 0);
            }
            free(temp_path);
        } else {
            //no arguments after '>'
            printError();
        }
    } else {
        //Too many '>' or too many arguments
        printError();
    }

} // end redirect function

int executeAllOtherCommands(char (*path)[MAX_PATH_SIZE], char *args[], int i, int arg_count)
{
    char *temp_path = malloc(MAX_PATH_SIZE);
    strcpy(temp_path, path[i]);
    strcat(temp_path, "/");
    strcat(temp_path, args[0]);

    bool redirection_flag = false;

    if (args[i + 1] != NULL) {
        if (strchr(args[i + 1], '>') != NULL)
        {
            redirection_flag = true;
        }
    }
    if (redirection_flag) {
        //printf("about to enter redirect\n");
        redirect(path, args, arg_count);
    } else if ((access(temp_path, X_OK) == 0))  {
        //printf("about to fork...\n");
        pid_t pid = fork();
        //printf("forked in executeAllOtherCommands\n");
        if (pid < 0) {
            printError();
            exit(1);
        } else if (pid == 0) { //child process executes command
            //printf("about to execute evecv()\n");
            execv(temp_path, args);
        } else { //parent process waits for child to complete
            int status;
            waitpid(pid, &status, 0);
        }
    } else { //file not found
        //printf("File not found: %s\n", temp_path);
        free(temp_path);
        return -1;
    }
    free(temp_path);
    return 0;
}

void executeCommands(char *args[], int arg_count, char (*path)[200]) {
    if (strcmp(args[0], "exit") == 0) {
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
                if (strcmp(args[i], "/bin") == 0) {
                    strcpy(path[num_of_paths], args[i]);
                    num_of_paths++;
                    continue;
                }
                else if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    strcat(cwd, "/");
                    if (args[i][0] != '/') { // If path doesn't start with '/'
                        strcat(cwd, args[i]);
                    } else { // If path starts with '/'
                        strcat(cwd, &args[i][1]);
                    }
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

void executeParallelCmd(char *args[], int arg_count, char (*path)[200]) {
    if (arg_count > 1) {
        parallel_cmd_flag = true;
        for (int i = 0; i < arg_count; i++) {
            //printf("Executing: '%s'\n", args[i]);

            char *command[MAX_ARGUMENTS];

            args[i][strcspn(args[i], "\n")] = '\0';
            int command_count = 0;
            char delimiter = ' ';

            char *token = strtok(args[i], &delimiter);
            while (token != NULL && arg_count < MAX_ARGUMENTS - 1) {
                while (*token != '\0' && isspace(*token)) {
                    token++;
                }
                char *end = token + strlen(token) - 1;
                while (end > token && isspace(*end)) {
                    *end-- = '\0';
                }

                //printf("Entered while loop for tokenizing in parallel command.\n");
                command[command_count] = token;
                command_count++;
                token = strtok(NULL, &delimiter);
            }
            command[command_count] = NULL;

//            printf("Tokenized by '%c'\n", delimiter);
//
//            for (int i = 0; i < command_count; i++)
//            {
//                printf("'%s'\n", command[i]);
//            }

            if (arg_count != 0 ) {
                executeCommands(command, command_count, path);
            }
        }
        exit(0);
    }
} // end executeParallelCmd()
