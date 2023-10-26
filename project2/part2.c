/*
 * MCP Ghost in the Shell v2.0
 *
 * Created on: October 25, 2023
 *     Author: jparke10
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "string_parser.h"

#define _GNU_SOURCE
#define BUF_SIZE 65536
#define LINE_MAX 2048

// Count of lines in file needed for pid array allocation
unsigned int count_lines(FILE* file);

void signaler(pid_t* pid_array, int size, int signal);

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
    char* buf = malloc(LINE_MAX);
    size_t buf_size = LINE_MAX;
    // w in wpid stands for "wait"
    pid_t wpid;
    pid_t* pid_array = malloc(sizeof(pid_t) * num_lines);
    sigset_t sigset;
    int sig;

    sigemptyset(&sigset);
    // add SIGUSR1 to blocked signals
    // when sigwait is called by children, they'll wait for blocked SIGUSR1
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    for (int i = 0; i < num_lines; i++) {
        // execute command vector parsing
        getline(&buf, &buf_size, file_in);
        args = str_filler(buf, " ");

        pid_array[i] = fork();
        if (pid_array[i] < 0) {
            fprintf(stderr, "Error in forking parent process\n");
            exit(EXIT_FAILURE);
        } else if (pid_array[i] == 0) {
            printf("child process %d waiting for SIGUSR1\n", getpid());
            // initially wait for SIGUSR1 signal
            sigwait(&sigset, &sig);
            if ((execvp(args.command_list[0], args.command_list)) == -1) {
                perror("Error launching child process");
            }
            exit(-1);
        }
        free_command_line(&args);
        memset(&args, 0, 0);
    }

    signaler(pid_array, num_lines, SIGUSR1);
    signaler(pid_array, num_lines, SIGSTOP);
    signaler(pid_array, num_lines, SIGCONT);

    // wait for all child processes to finish
    while ((wpid = waitpid(-1, NULL, 0)) > 0);
    printf("All processes finished\n");
    free(pid_array);
    free(buf);
    fclose(file_in);
    exit(EXIT_SUCCESS);
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

void signaler(pid_t* pid_array, int size, int signal) {
    // wait for some clarity on what program is doing
    sleep(3);
    for (int i = 0; i < size; i++) {
        printf("parent %d signaling %s to child %d\n",
                getpid(), strsignal(signal), pid_array[i]);
        kill(pid_array[i], signal);
    }
}