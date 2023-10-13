#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_parser.h"
#include "string_parser.h"

#define _GNU_SOURCE
#define LINE_MAX 128
#define INTERACTIVE_STREAM stdin
#define INTERACTIVE_MODE 0
#define FILE_MODE 1

void shell(FILE* stream, int flag) {
    command_line command_buffer;
    command_line parameter_buffer;
    size_t len = LINE_MAX;
    char* line_buf = malloc(len);
    if (flag == INTERACTIVE_MODE)
        printf(">>> ");
    while (getline(&line_buf, &len, stream) != EOF) {
        command_buffer = str_filler(line_buf, ";");
        for (int i = 0; command_buffer.command_list[i] != NULL; i++) {
            parameter_buffer = str_filler(command_buffer.command_list[i], " ");
            // special command - terminate shell if exit is sent
            // getline will hit EOF if stdin is closed
            if (strcmp(parameter_buffer.command_list[0], "exit") == 0) {
                fclose(stdin);
            } else
                findAndExecute(&parameter_buffer);
            free_command_line(&parameter_buffer);
            memset(&parameter_buffer, 0, 0);
        }
        free_command_line(&command_buffer);
        memset(&command_buffer, 0, 0);
        if (flag == INTERACTIVE_MODE)
            printf(">>> ");
    }
    if (flag == INTERACTIVE_MODE)
        printf("\n");
    free(line_buf);
}

int main(int argc, char** argv) {
    // checking for command line argument
    switch (argc) {
        case 1:
            shell(INTERACTIVE_STREAM, INTERACTIVE_MODE);
            exit(EXIT_SUCCESS);
        case 3:
            if (strcmp(argv[1], "-f") == 0) {
                FILE* execute = fopen(argv[2], "r");
                shell(execute, FILE_MODE);
                fclose(execute);
                exit(EXIT_SUCCESS);
            }
    }
    fprintf(stderr, "Usage: %s [-f] [FILE]\n", argv[0]);
    exit(EXIT_FAILURE);
}