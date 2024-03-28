#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "commands.h"

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10
#define MAX_PATH_SIZE 200
#define MAX_PATH_COUNT 100

char path[MAX_PATH_COUNT][MAX_PATH_SIZE] = {"/bin"};

void interactiveMode()
{
	printf("wish> ");
    char input[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGUMENTS];
    while (1)
    {
        fgets(input, MAX_COMMAND_LENGTH, stdin);
        input[strcspn(input, "\n")] = '\0';
        bool parallel_command_flag = false;
        int arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARGUMENTS - 1) {
            args[arg_count] = token;
            if (strcmp(args[arg_count], "&") == 0)
            {
                parallel_command_flag = true;
            }
            arg_count++;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        if (parallel_command_flag == true)
        {
            executeParallelCmd(args, arg_count, path);
        }
        else if (arg_count != 0 ) {
            executeCommands(args, arg_count, path);
        }
    }
}

void batchMode(char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printError();
        exit(1);
	}
	else {
        char line[MAX_COMMAND_LENGTH];
        char *args[MAX_ARGUMENTS];
        while (fgets(line, MAX_COMMAND_LENGTH, file) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            bool parallel_command_flag = false;
            int arg_count = 0;
            char *token = strtok(line, " ");
            while (token != NULL && arg_count < MAX_ARGUMENTS - 1) {
                args[arg_count] = token;
                if (strcmp(args[arg_count], "&") == 0)
                {
                    parallel_command_flag = true;
                }
                arg_count++;
                token = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            if (parallel_command_flag == true)
            {
                executeParallelCmd(args, arg_count, path);
            }
            else if (arg_count != 0 ) {
                executeCommands(args, arg_count, path);
            }
        }
        fclose(file);
    }
}