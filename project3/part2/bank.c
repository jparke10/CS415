#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "account.h"
#include "string_parser.h"

#define NUM_THREADS 10
#define CHUNK_SIZE(X) X / NUM_THREADS

typedef struct {
    FILE* file_in;
    unsigned int chunk_lines;
    char* local_buf;
    long chunk_offset;
} file_chunk;

size_t buf_size = LINE_MAX;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
account* account_array = NULL;
char* file_reading = NULL;
unsigned int num_accounts = 0;

void* process_transaction(void* arg) {
    file_chunk* chunk = (file_chunk*)arg;
    unsigned int line_counter = 0;
    chunk->file_in = fopen(file_reading, "r");
    fseek(chunk->file_in, chunk->chunk_offset, SEEK_SET);

    while (line_counter < chunk->chunk_lines) {
        // get line and parse it
        pthread_mutex_lock(&file_mutex);
        ssize_t read = getline(&(chunk->local_buf), &buf_size, chunk->file_in);
        pthread_mutex_unlock(&file_mutex);
        if (read == EOF) {
            pthread_exit(0);
        }
        command_line input = str_filler(chunk->local_buf, " ");
        char* type = input.command_list[0];
        char* account = input.command_list[1];
        char* password = input.command_list[2];

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

        if (*type == 'T') {
            int destination_index = -1;
            double amount = atof(input.command_list[4]);
            for (int i = 0; i < num_accounts; i++) {
                if (strcmp(input.command_list[3], account_array[i].account_number) == 0) {
                    destination_index = i;
                    break;
                }
            }
            pthread_mutex_lock(&(account_array[account_index].ac_lock));
            pthread_mutex_lock(&(account_array[destination_index].ac_lock));
            account_array[account_index].balance -= amount;
            account_array[account_index].transaction_tracter += amount;
            account_array[destination_index].balance += amount;
            pthread_mutex_unlock(&account_array[account_index].ac_lock);
            pthread_mutex_unlock(&account_array[destination_index].ac_lock);
        } else if (*type == 'C') {
        } else if (*type == 'D') {
            double amount = atof(input.command_list[3]);
            pthread_mutex_lock(&(account_array[account_index].ac_lock));
            account_array[account_index].balance += amount;
            account_array[account_index].transaction_tracter += amount;
            pthread_mutex_unlock(&account_array[account_index].ac_lock);
        } else if (*type == 'W') {
            double amount = atof(input.command_list[3]);
            pthread_mutex_lock(&(account_array[account_index].ac_lock));
            account_array[account_index].balance -= amount;
            account_array[account_index].transaction_tracter += amount;
            pthread_mutex_unlock(&account_array[account_index].ac_lock);
        }
        free_command_line(&input);
        memset(&input, 0, 0);
        line_counter++;
    }
    pthread_exit(0);
}

void* update_balance(void* arg) {
    for (int i = 0; i < num_accounts; i++) {
        pthread_mutex_lock(&(account_array[i].ac_lock));
        account_array[i].balance += account_array[i].transaction_tracter *
                                    account_array[i].reward_rate;
        account_array[i].transaction_tracter = 0.;
        pthread_mutex_unlock(&(account_array[i].ac_lock));
        FILE* acc = fopen(account_array[i].out_file, "a");
        fprintf(acc, "Current Balance:\t%.2f\n", account_array[i].balance);
        fclose(acc);
    }
    pthread_exit(0);
}

void usage(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

unsigned int count_lines(FILE* file) {
    long start_index = ftell(file);
    char buf[65536];
    unsigned int count = 0;
    while (1) {
        size_t read = fread(buf, sizeof(char), 65536, file);
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
    // return cursor to original position
    fseek(file, start_index, SEEK_SET);
    return ++count;
}

int main(int argc, char** argv) {
    usage(argc, argv);
    FILE* file_in = fopen(argv[1], "r");
    if (file_in == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    file_reading = argv[1];

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
        memcpy(account_array[i].account_number, buf, 16);
        snprintf(account_array[i].out_file, 63, "./output/%s.txt",
                 account_array[i].account_number);
        FILE* acc = fopen(account_array[i].out_file, "w");
        fprintf(acc, "account %d:\n", i);
        fclose(acc);
        // password line
        getline(&buf, &buf_size, file_in);
        memcpy(account_array[i].password, buf, 8);
        // initial balance
        getline(&buf, &buf_size, file_in);
        account_array[i].balance = atof(buf);
        // reward rate
        getline(&buf, &buf_size, file_in);
        account_array[i].reward_rate = atof(buf);
    }
    printf("done\n");

    printf("Chunking file %s for processing...", argv[1]);
    file_chunk* file = malloc(sizeof(file_chunk) * NUM_THREADS);
    // count number of transactions to process
    unsigned int transactions = count_lines(file_in);
    unsigned short extras = transactions % NUM_THREADS;
    long transactions_start = ftell(file_in);
    // populate file input struct
    for (int i = 0; i < NUM_THREADS; i++) {
        file[i].file_in = fopen(argv[1], "r");
        fseek(file[i].file_in, transactions_start, SEEK_SET);
        file[i].chunk_offset = 0;
        file[i].chunk_lines = 0;
        file[i].chunk_lines = CHUNK_SIZE(transactions);
        file[i].local_buf = malloc(LINE_MAX);
        memset(file[i].local_buf, 0, LINE_MAX);
        if (extras > 0) {
            file[i].chunk_lines += 1;
            extras -= 1;
        }

        size_t lines_offset = 0;
        for (int j = 0; j < i; j++) {
            lines_offset += file[j].chunk_lines;
        }
        // seek file pointer to line offset of chunk
        // probably slower than single threaded solution...
        for (size_t j = 0; j < lines_offset; j++) {
            getline(&buf, &buf_size, file[i].file_in);
        }
        file[i].chunk_offset = ftell(file[i].file_in);
        fclose(file[i].file_in);
    }
    printf("done\n");

    pthread_t workers[NUM_THREADS];
    pthread_t bank_thread;
    printf("Processing transactions from %s...", argv[1]);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&(workers[i]), NULL, process_transaction, &(file[i]));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(workers[i], 0);
    }
    printf("done\n");

    printf("Updating balances with reward rate...");
    pthread_create(&bank_thread, NULL, update_balance, NULL);
    pthread_join(bank_thread, 0);
    printf("done\n");

    FILE* total_transactions = fopen("output.txt", "w");
    for (int i = 0; i < num_accounts; i++) {
        fprintf(total_transactions, "%d balance:\t%.2f\n\n", i, account_array[i].balance);
    }

    fclose(total_transactions);
    fclose(file_in);
    free(account_array);
    free(buf);
}