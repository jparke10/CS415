#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include "account.h"

void* process_transaction(void* arg) {

}

void* update_balance(void* arg) {

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
    const unsigned int num_accounts = atoi(buf);
    if (num_accounts == 0) {
        fprintf(stderr, "Error: Invalid number of accounts in input file\n");
        exit(EXIT_FAILURE);
    }

    account* account_array = malloc(sizeof(account) * num_accounts);
    for (int i = 0; i < num_accounts; i++) {
        // index line
        getline(&buf, &buf_size, file_in);
        // account number line
        getline(&buf, &buf_size, file_in);
        strncpy(account_array[i].account_number, buf, 17);
        // password line
        getline(&buf, &buf_size, file_in);
        strncpy(account_array[i].password, buf, 9);
        // initial balance
        getline(&buf, &buf_size, file_in);
        account_array[i].balance = atof(buf);
        // reward rate
        getline(&buf, &buf_size, file_in);
        account_array[i].reward_rate = atof(buf);
    }

    
}

void usage(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}