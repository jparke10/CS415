/*
 * MCP Ghost in the Shell v1.0
 *
 * Created on: October 23, 2023
 *     Author: jparke10
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "MCP.h"

int main(int argc, char** argv) {
    usage(argc, argv);
    FILE* file_in = fopen(argv[1], "r");
    if (file_in == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    command_line args;
    unsigned int num_lines = count_lines(file_in);
    char* buf = malloc(LINE_MAX);
    size_t buf_size = LINE_MAX;
    // w in wpid stands for "wait"
    pid_t wpid;
    pid_t* pid_array = malloc(sizeof(pid_t) * num_lines);

    for (int i = 0; i < num_lines; i++) {
        // execute command vector parsing
        getline(&buf, &buf_size, file_in);
        args = str_filler(buf, " ");

        pid_array[i] = fork();
        if (pid_array[i] < 0) {
            perror("Error while forking parent process");
            exit(EXIT_FAILURE);
        } else if (pid_array[i] == 0) {
            if ((execvp(args.command_list[0], args.command_list)) == -1) {
                perror("Error launching child process");
            }
            exit(-1);
        }
        free_command_line(&args);
        memset(&args, 0, 0);
    }
    // wait for all child processes to finish
    while ((wpid = wait(NULL)) > 0);
    free(pid_array);
    free(buf);
    fclose(file_in);
    exit(EXIT_SUCCESS);
}

void usage(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

unsigned int count_lines(FILE* file) {
    char buf[BUF_SIZE];
    unsigned int count = 0;
    while (1) {
        size_t read = fread(buf, sizeof(char), BUF_SIZE, file);
        if (ferror(file)) {
            fprintf(stderr, "Error getting number of lines in file\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < read; i++) {
            if (buf[i] == '\n')
                count++;
        }
        if(feof(file))
            break;
    }
    fseek(file, 0, SEEK_SET);
    return ++count;
}

int count_token (char* buf, const char* delim) {
	// check for NULL string
	// returning count 1 will account for lone NULL token in command_list
	if (buf == NULL) {
		return 1;
	}
	// nondestructive by duplicating input string
	char* str = strdup(buf);
	char* token, *saveptr;
	int num_token = 0;
	token = strtok_r(str, delim, &saveptr);

	while (token != NULL) {
		// strings ending with delimiter will get an extra token containing '\n'
		// accounted for by stripping '\n' in str_filler
		num_token++;
		token = strtok_r(NULL, delim, &saveptr);
	}
	free(str);
	// account NULL for last token
	return ++num_token;
}

command_line str_filler (char* buf, const char* delim) {
	command_line to_fill;
	char* str = strdup(buf);
	char* end_str, *end_token;
	int i = 0;

	char* str_stripped = strtok_r(str, "\n", &end_str);
	to_fill.num_token = count_token(str_stripped, delim);

	to_fill.command_list = malloc(sizeof(char*) * to_fill.num_token);

	char* token = strtok_r(str, delim, &end_token);
	while (token != NULL) {
		// length of token plus NULL character
		to_fill.command_list[i] = malloc(strlen(token) + 1);
		strcpy(to_fill.command_list[i], token);
		token = strtok_r(NULL, delim, &end_token);
		i++;
	}
	to_fill.command_list[to_fill.num_token - 1] = NULL;
	
	free(str);
	return to_fill;
}

void free_command_line(command_line* command) {
	for (int i = 0; i < command->num_token; i++)
		free(command->command_list[i]);
	free(command->command_list);
}