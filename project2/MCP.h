/*
 * mcp.h
 * 
 * Created on: Nov 2, 2023
 *     Author: jparke10
 * 
 * Purpose: Consolidation of function prototypes and struct definitions
 * used by MCP 'Ghost in the Shell' project. Contains struct definition
 * and string parsing function prototypes from string_parser.h authored
 * by gguan and Monil in Nov 2020, as well as helper function prototypes
 * for MCP authored by jparke10.
 * 
 */

#ifndef MCP_H_
#define MCP_H_

// Useful definitions
#define _GNU_SOURCE
#define BUF_SIZE 65536 // Reading of input file
#define LINE_MAX 2048 // Subsequent reading of individual lines of input file

/*
 * String parsing struct and prototypes from string_parser.h 
 * I could have kept these in string_parser.h, but I opted to
 * consolidate the number of files uploaded for this project
 */

typedef struct {
    char** command_list;
    int num_token;
} command_line;

// This function returns the number of tokens needed for the string array
// based on the delimiter
int count_token (char* buf, const char* delim);

// This function can tokenize a string to token arrays base on a specified delimiter,
// it returns a struct variable
command_line str_filler (char* buf, const char* delim);


// This function safely frees all the tokens within the array
void free_command_line(command_line* command);

/* ---------------------------------------------------------------------------- */

/* Useful function prototypes for MCP */

// Check input 
void usage(int argc, char** argv);

// Count the number of lines (processes to be run) in input.txt
unsigned int count_lines(FILE* file);

// Communicates a signal to each process in the pid array, useful
// for part 2 of MCP
void signaler(pid_t* pid_array, int size, int signal);

// Endless loop to check if it's time to start a new child process,
// and if it is, do so
void scheduler_loop(pid_t* pid_array, const unsigned int size);

void print_status(const pid_t pid);

#endif