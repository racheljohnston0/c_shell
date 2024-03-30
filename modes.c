#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

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
        char delimiter = ' ';

        if (strchr(input, '&') != NULL)
        {
            delimiter = '&';
            parallel_command_flag = true;
        }

        char *token = strtok(input, &delimiter);
        while (token != NULL && arg_count < MAX_ARGUMENTS - 1) {

            while (*token != '\0' && isspace(*token)) {
                token++;
            }
            char *end = token + strlen(token) - 1;
            while (end > token && isspace(*end)) {
                *end-- = '\0';
            }

            printf("Entered while loop for tokenizing.\n");
            args[arg_count] = token;
            arg_count++;
            token = strtok(NULL, &delimiter);
        }
        args[arg_count] = NULL;

        printf("Tokenized by '%c'\n", delimiter);

        for (int i = 0; i < arg_count; i++)
        {
            printf("%s\n", args[i]);
        }

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
            char delimiter = ' ';

            if (strchr(line, '&') != NULL)
            {
                delimiter = '&';
                parallel_command_flag = true;
            }

            char *token = strtok(line, &delimiter);
            while (token != NULL && arg_count < MAX_ARGUMENTS - 1) {

                while (*token != '\0' && isspace(*token)) {
                    token++;
                }
                char *end = token + strlen(token) - 1;
                while (end > token && isspace(*end)) {
                    *end-- = '\0';
                }

                args[arg_count] = token;
                arg_count++;
                token = strtok(NULL, &delimiter);
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