#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "account.h"
#include "string_parser.h"

unsigned int num_accounts = 0;
account* account_array = NULL;

void* process_transaction(void* arg) {
    command_line* input = (command_line*)arg;
    char* type = input->command_list[0];
    char* account = input->command_list[1];
    char* password = input->command_list[2];
    int account_index = -1;
    for (int i = 0; i < num_accounts; i++) {
        if (strcmp(account, account_array[i].account_number) == 0) {
            account_index = i;
            break;
        }
    }
    // password validation
    if (strcmp(password, account_array[account_index].password) != 0) {
        return NULL;
    }

    if (strcmp(type, "T") == 0) {
        int destination_index = -1;
        double amount = atof(input->command_list[4]);
        for (int i = 0; i < num_accounts; i++) {
            if (strcmp(input->command_list[3], account_array[i].account_number) == 0) {
                destination_index = i;
                break;
            }
        }
        account_array[account_index].balance -= amount;
        account_array[account_index].transaction_tracter += amount;
        account_array[destination_index].balance += amount;
    } else if (strcmp(type, "C") == 0) {
    } else if (strcmp(type, "D") == 0) {
        double amount = atof(input->command_list[3]);
        account_array[account_index].balance += amount;
        account_array[account_index].transaction_tracter += amount;
    } else if (strcmp(type, "W") == 0) {
        double amount = atof(input->command_list[3]);
        account_array[account_index].balance -= amount;
        account_array[account_index].transaction_tracter += amount;
    }
    return NULL;
}

void* update_balance(void* arg) {
    for (int i = 0; i < num_accounts; i++) {
        account_array[i].balance += account_array[i].transaction_tracter *
                                    account_array[i].reward_rate;
        FILE* acc = fopen(account_array[i].out_file, "a");
        fprintf(acc, "Current Balance:\t%.2f\n", account_array[i].balance);
    }
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
    char* buf = (char*)malloc(buf_size);

    getline(&buf, &buf_size, file_in);
    num_accounts = atoi(buf);
    if (num_accounts == 0) {
        fprintf(stderr, "Error: Invalid number of accounts in input file\n");
        exit(EXIT_FAILURE);
    }
    printf("DuckBank accounts: %d\n", num_accounts);

    account_array = (account*)malloc(sizeof(account) * num_accounts);
    printf("Populating accounts...");
    mkdir("output", 0777);
    for (int i = 0; i < num_accounts; i++) {
        // null initializations of each account
        // helps prevent memory errors
        memset(account_array[i].account_number, 0, 17);
        memset(account_array[i].password, 0, 9);
        memset(account_array[i].out_file, 0, 64);
        account_array[i].balance = 0.;
        account_array[i].reward_rate = 0.;
        account_array[i].transaction_tracter = 0.;
        pthread_mutex_init(&(account_array[i].ac_lock), NULL);
        // index line
        getline(&buf, &buf_size, file_in);
        // account number line
        getline(&buf, &buf_size, file_in);
        strncpy(account_array[i].account_number, buf, 16);
        snprintf(account_array[i].out_file, 63, "./output/%s.txt",
                 account_array[i].account_number);
        FILE* acc = fopen(account_array[i].out_file, "w");
        fprintf(acc, "account %d:\n", i);
        fclose(acc);
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
    printf("done\n");

    command_line transaction;
    printf("Processing transactions from %s...", argv[1]);
    while (getline(&buf, &buf_size, file_in) != EOF) {
        transaction = str_filler(buf, " ");
        process_transaction(&transaction);
        free_command_line(&transaction);
        memset(&transaction, 0, 0);
    }
    printf("done\n");

    printf("Updating balances with reward rate...");
    update_balance(NULL);
    printf("done\n");

    FILE* transactions = fopen("output.txt", "w");
    for (int i = 0; i < num_accounts; i++) {
        fprintf(transactions, "%d balance:\t%.2f\n\n", i, account_array[i].balance);
    }

    fclose(transactions);
    free(buf);
}