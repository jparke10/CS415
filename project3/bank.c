#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include "account.h"
#include "string_parser.h"

unsigned int num_accounts = 0;
account* account_array;

void* process_transaction(command_line* arg) {
    char* type = arg->command_list[0];
    char* account = arg->command_list[1];
    char* password = arg->command_list[2];
    int account_index = -1;
    for (int i = 0; i < num_accounts; i++) {
        if (strcmp(account, account_array[i].account_number) == 0) {
            account_index = i;
            break;
        }
    }
    // password validation
    if (strcmp(password, account_array[account_index].password) != 0) {
        return;
    }

    if (type == "T") {

    } else if (type == "C") {

    } else if (type == "D") {

    } else if (type == "w") {

    }
}

void* update_balance(void* arg) {

}

void usage(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    usage(argc, argv);
    FILE* file_in = fopen(argv[1], "r");
    if (file_in == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    size_t buf_size = LINE_MAX;
    char* buf = malloc(buf_size);

    getline(&buf, &buf_size, file_in);
    num_accounts = atoi(buf);
    if (num_accounts == 0) {
        fprintf(stderr, "Error: Invalid number of accounts in input file\n");
        exit(EXIT_FAILURE);
    }

    account_array = (account*)malloc(sizeof(account) * num_accounts);
    for (int i = 0; i < num_accounts; i++) {
        // index line
        getline(&buf, &buf_size, file_in);
        // account number line
        getline(&buf, &buf_size, file_in);
        strncpy(account_array[i].account_number, buf, 16);
        strncat(account_array[i].account_number, "\0", 1);
        // password line
        getline(&buf, &buf_size, file_in);
        strncpy(account_array[i].password, buf, 8);
        strncat(account_array[i].password, "\0", 1);
        // initial balance
        getline(&buf, &buf_size, file_in);
        account_array[i].balance = atof(buf);
        // reward rate
        getline(&buf, &buf_size, file_in);
        account_array[i].reward_rate = atof(buf);
    }

    command_line transaction;
    while (getline(&buf, &buf_size, file_in) != EOF) {
        transaction = str_filler(buf, " ");
        process_transaction(&transaction);
        free_command_line(&transaction);
        memset(&transaction, 0, 0);
    }
}