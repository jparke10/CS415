#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GNU_SOURCE
#define LINE_MAX 128

void shellFileMode(char* filename) {

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

        }
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