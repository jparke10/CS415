#ifndef COMMAND_PARSER_H_
#define COMMAND_PARSER_H_

#include <stdio.h>
#include <string.h>
#include "string_parser.h"
#include "command.h"

#define _GNU_SOURCE

typedef struct {
    void (*func_no_param)(void);
    void (*func_one_param)(char* );
    void (*func_two_param)(char* , char* );
} func_ptr;

void findAndExecute(command_line* tokens);

#endif