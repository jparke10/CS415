#include "command_parser.h"

const char* command_names[] = {
    "ls",
    "pwd",
    "mkdir",
    "cd",
    "cp",
    "mv",
    "rm",
    "cat"
};

const int num_commands = sizeof(command_names) / sizeof(char* );

const func_ptr func_ptrs[] = {
    {.func_no_param = &listDir},
    {.func_no_param = &showCurrentDir},
    {.func_one_param = &makeDir},
    {.func_one_param = &changeDir},
    {.func_two_param = &copyFile},
    {.func_two_param = &moveFile},
    {.func_one_param = &deleteFile},
    {.func_one_param = &displayFile}
};

void findAndExecute(command_line* tokens) {
    // boolean for printed error messages
    char command_found = 0;
    for (int i = 0; i < num_commands; i++) {
        if (strcmp(tokens->command_list[0], command_names[i]) == 0) {
            command_found++;
            // account NULL for last token - "2-based" numbering
            // 2 would be no parameters, 3 - 1 parameter, etc
            switch (tokens->num_token) {
                case 2:
                    if (func_ptrs[i].func_no_param != NULL)
                        return func_ptrs[i].func_no_param();
                case 3:
                    if (func_ptrs[i].func_one_param != NULL)
                        return func_ptrs[i].func_one_param(tokens->command_list[1]);
                case 4:
                    if (func_ptrs[i].func_two_param != NULL)
                        return func_ptrs[i].func_two_param(tokens->command_list[1],
                                                           tokens->command_list[2]);
            }
        }
    }
    // if not returned from switch, print appropriate error message
    if (command_found)
        printf("Error! Unsupported parameters for command: %s\n", tokens->command_list[0]);
    else
        printf("Error! Unrecognized command: %s\n", tokens->command_list[0]);
    return;
}