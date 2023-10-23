#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "string_parser.h"

#define _GNU_SOURCE
#define LINE_MAX 2048

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE* file_in = fopen(argv[1], "r");
    if (file_in == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    command_line lines, args;
    char* buf;
    size_t buf_size;
    fseek(file_in, 0, SEEK_END);
    buf_size = ftell(file_in);
    fseek(file_in, 0, SEEK_SET);
    buf = malloc(buf_size);
    fread(buf, sizeof(char), buf_size, file_in);
    size_t num_lines = count_token(buf, "\n") - 1;
    printf("%d\n", num_lines);
    pid_t* pid_array = malloc(sizeof(pid_t) * num_lines);
    exit(EXIT_SUCCESS);
}