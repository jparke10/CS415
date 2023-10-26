#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "string_parser.h"

#define _GNU_SOURCE
#define BUF_SIZE 65536
#define LINE_MAX 2048

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
    command_line args;
    unsigned int num_lines = count_lines(file_in);
    pid_t wpid;
    pid_t* pid_array = malloc(sizeof(pid_t) * num_lines);
    char* buf = malloc(LINE_MAX);
    size_t buf_size = LINE_MAX;
    for (int i = 0; i < num_lines; i++) {
        getline(&buf, &buf_size, file_in);
        args = str_filler(buf, " ");
        pid_array[i] = fork();
        if (pid_array[i] < 0) {
            fprintf(stderr, "Error in fork\n");
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
    while ((wpid = wait(NULL)) > 0);
    free(pid_array);
    free(buf);
    fclose(file_in);
    exit(EXIT_SUCCESS);
}