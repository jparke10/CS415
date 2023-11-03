/*
 * MCP Ghost in the Shell v2.0
 *
 * Created on: October 25, 2023
 *     Author: jparke10
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "MCP.h"

// alarm flag, says if new process should be started
// this iteration of the scheduler
static volatile sig_atomic_t start_next = 1;

int main(int argc, char** argv) {
    FILE* file_in;
    usage(argc, argv, file_in);

    command_line args;
    const unsigned int num_processes = count_lines(file_in);
    char* buf = malloc(LINE_MAX);
    size_t buf_size = LINE_MAX;
    pid_t* pid_array = malloc(sizeof(pid_t) * num_processes);

    for (int i = 0; i < num_processes; i++) {
        // execute command vector parsing
        getline(&buf, &buf_size, file_in);
        args = str_filler(buf, " ");

        pid_array[i] = fork();
        if (pid_array[i] < 0) {
            fprintf(stderr, "Error in forking parent process\n");
            exit(EXIT_FAILURE);
        } else if (pid_array[i] == 0) {
            // when process is created, immediately pause
            // scheduler can then choose which process to continue
            raise(SIGSTOP);
            if ((execvp(args.command_list[0], args.command_list)) == -1) {
                perror("Error launching child process");
            }
            exit(-1);
        }
        free_command_line(&args);
        memset(&args, 0, 0);
    }

    signal(SIGALRM, handle_alarm);
    scheduler_loop(pid_array, num_processes);
    free(pid_array);
    free(buf);
    fclose(file_in);
    exit(EXIT_SUCCESS);
}

void handle_alarm (int signum) {
    start_next = 1;
}

void signaler(pid_t* pid_array, int size, int signal) {
    sleep(3);
    for (int i = 0; i < size; i++) {
        kill(pid_array[i], signal);
    }
}

void usage(int argc, char** argv, FILE* to_open) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    to_open = fopen(argv[1], "r");
    if (to_open == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", argv[1]);
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

void scheduler_loop(pid_t* pid_array, const unsigned int size) {
    // first, a way to block scheduler while waiting for SIGALRM
    // ensures child process gets its fair time slice
    sigset_t sigset;
    int sig;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    int process_exited[size];
    for (int i = 0; i < size; i++) {
        process_exited[i] = 0;
    }

    unsigned int processes_finished = 0;
    int status;
    unsigned int process_index = 0;
    pid_t wpid;

    // start first process, then enter loop
    kill(pid_array[0], SIGCONT);
    while (1) {
        // if current process has not exited, schedule the next one
        // wait until alarm handler has finished to continue scheduler
        if (process_exited[process_index] == 0) {
            alarm(2);
            sigwait(&sigset, &sig);
        }

    }
}