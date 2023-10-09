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
    for (int i = 0; i < num_commands; i++) {
        if (strcmp(tokens->command_list[0], command_names[i]) == 0) {
            // account NULL for last token - "2-based" numbering
            // 2 would be no parameters, 3 - 1 parameter, etc
            switch (tokens->num_token) {
                case 2:
                    if (func_ptrs[i].func_no_param == NULL) {
                        printf("Error! Unsupported parameters for command: %s\n",
                               tokens->command_list[0]);
                        return;
                    } else
                        return func_ptrs[i].func_no_param();
                case 3:
                    if (func_ptrs[i].func_one_param == NULL) {
                        printf("Error! Unsupported parameters for command: %s\n",
                               tokens->command_list[0]);
                        return;
                    } else
                        return func_ptrs[i].func_one_param(tokens->command_list[1]);
                case 4:
                    if (func_ptrs[i].func_two_param == NULL) {
                        printf("Error! Unsupported parameters for command: %s\n",
                               tokens->command_list[0]);
                        return;
                    } else
                        return func_ptrs[i].func_two_param(tokens->command_list[1],
                                                           tokens->command_list[2]);
                default:
                    printf("Error! Unsupported parameters for command: %s\n",
                           tokens->command_list[0]);
            }
        }
    }
    // if command not found in loop, it doesn't exist
    printf("Error! Unrecognized command: %s\n", tokens->command_list[0]);
    return;
}