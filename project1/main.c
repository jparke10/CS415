#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_parser.h"
#include "string_parser.h"

#define _GNU_SOURCE
#define LINE_MAX 128

void shellFileMode(char* filename) {
    FILE* to_parse = fopen(filename, "r");
    if (to_parse == NULL) {
        fprintf(stderr, "File %s could not be opened\n", filename);
        fprintf(stderr, "Check file path and ensure file has read permission\n");
        exit(EXIT_FAILURE);
    }
    command_line command_buffer;
    command_line parameter_buffer;
    size_t len = LINE_MAX;
    char* line_buf = malloc(len);
    while (getline(&line_buf, &len, to_parse) != EOF) {
        command_buffer = str_filler(line_buf, ";");
        for (int i = 0; command_buffer.command_list[i] != NULL; i++) {
            parameter_buffer = str_filler(command_buffer.command_list[i], " ");
            findAndExecute(&parameter_buffer);
            free_command_line(&parameter_buffer);
            memset(&parameter_buffer, 0, 0);
        }
        free_command_line(&command_buffer);
        memset(&command_buffer, 0, 0);
    }
    free(line_buf);
}

void shellInteractiveMode() {
    command_line command_buffer;
    command_line parameter_buffer;
    size_t len = LINE_MAX;
    char* line_buf = malloc(len);

    printf(">>> ");
    while (getline(&line_buf, &len, stdin) != EOF) {
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
        printf(">>> ");
    }
    printf("\n");
    free(line_buf);
}

int main(int argc, char** argv) {
    // checking for command line argument
    switch (argc) {
        case 1:
            shellInteractiveMode();
            exit(EXIT_SUCCESS);
        case 3:
            if (strcmp(argv[1], "-f") == 0) {
                shellFileMode(argv[2]);
                exit(EXIT_SUCCESS);
            }
    }
    fprintf(stderr, "Usage: %s [-f] [FILE]\n", argv[0]);
    exit(EXIT_FAILURE);
}