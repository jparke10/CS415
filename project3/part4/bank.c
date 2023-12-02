#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include "account.h"
#include "string_parser.h"

#define NUM_THREADS 10
#define REQUEST_THRESHOLD 5000
#define CHUNK_SIZE(X) X / NUM_THREADS

// Struct for each file chunk
// Contains starting location of the chunk and its own file stream
// Allows starting file read at the chunk's starting location
typedef struct {
    FILE* file_in;
    unsigned int chunk_lines;
    char* local_buf;
    unsigned int lines_offset;
} file_chunk;

account* shared;
size_t buf_size = LINE_MAX;
pthread_barrier_t sync_workers;
pthread_cond_t updating_balance, balance_updated;
pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;
account* account_array = NULL;
// global file_reading is equivalent to argv[1]
char* file_reading = NULL;
unsigned int num_accounts = 0;
// counter for the number of transaction requests, used for threshold/balancing
unsigned int transactions_processed = 0;
// used to terminate bank thread after all workers completed
unsigned short final_update = 0;
// printed by parent thread
unsigned int num_updates = 0;
// file pointer position immediately after account block
long transactions_start = 0;
// for final output file printing
unsigned short is_parent = 0;

void* process_transaction(void* arg) {
    file_chunk* chunk = (file_chunk*)arg;
    command_line current_line;
    chunk->file_in = fopen(file_reading, "r");
    fseek(chunk->file_in, transactions_start, SEEK_SET);
    for (unsigned int i = 0; i < chunk->lines_offset; i++)
        getline(&(chunk->local_buf), &buf_size, chunk->file_in);
    
    // all threads process concurrently after offsets are lined up
    pthread_barrier_wait(&sync_workers);
    for (unsigned int i = 0; i < chunk->chunk_lines; i++) {
        // get line and parse it
        ssize_t read = getline(&(chunk->local_buf), &buf_size, chunk->file_in);
        if (read == EOF) {
            break;
        }
        current_line = str_filler(chunk->local_buf, " ");
        char* type = current_line.command_list[0];
        char* account = current_line.command_list[1];
        char* password = current_line.command_list[2];

        int account_index = -1;
        for (int i = 0; i < num_accounts; i++) {
            if (strncmp(account, account_array[i].account_number, 16) == 0) {
                account_index = i;
                break;
            }
        }

        // password validation
        // if password is invalid, free command line and parse next line
        if (strncmp(password, account_array[account_index].password, 8) != 0) {
            free_command_line(&current_line);
            memset(&current_line, 0, 0);
            continue;
        }

        // handler for balance checks, which do not contribute to requests
        // just parse next line
        if (*type == 'C') {
            free_command_line(&current_line);
            memset(&current_line, 0, 0);
            continue;
        }

        pthread_mutex_lock(&request_mutex);
        while (transactions_processed >= REQUEST_THRESHOLD) {
            printf("%d number reached, pausing\n", syscall(SYS_gettid));
            pthread_cond_wait(&balance_updated, &request_mutex);
        }
        pthread_mutex_unlock(&request_mutex);

        if (*type == 'T') {
            int destination_index = -1;
            double amount = atof(current_line.command_list[4]);
            for (int i = 0; i < num_accounts; i++) {
                if (strcmp(current_line.command_list[3], account_array[i].account_number) == 0) {
                    destination_index = i;
                    break;
                }
            }
            pthread_mutex_lock(&(account_array[account_index].ac_lock));
            account_array[account_index].balance -= amount;
            account_array[account_index].transaction_tracter += amount;
            pthread_mutex_unlock(&account_array[account_index].ac_lock);
            pthread_mutex_lock(&(account_array[destination_index].ac_lock));
            account_array[destination_index].balance += amount;
            pthread_mutex_unlock(&account_array[destination_index].ac_lock);
        } else if (*type == 'D') {
            double amount = atof(current_line.command_list[3]);
            pthread_mutex_lock(&(account_array[account_index].ac_lock));
            account_array[account_index].balance += amount;
            account_array[account_index].transaction_tracter += amount;
            pthread_mutex_unlock(&account_array[account_index].ac_lock);
        } else if (*type == 'W') {
            double amount = atof(current_line.command_list[3]);
            pthread_mutex_lock(&(account_array[account_index].ac_lock));
            account_array[account_index].balance -= amount;
            account_array[account_index].transaction_tracter += amount;
            pthread_mutex_unlock(&account_array[account_index].ac_lock);
        }

        pthread_mutex_lock(&request_mutex);
        transactions_processed++;
        if (transactions_processed == REQUEST_THRESHOLD) {
            pthread_cond_signal(&updating_balance);
        }
        pthread_mutex_unlock(&request_mutex);

        free_command_line(&(current_line));
        memset(&(current_line), 0, 0);
    }
    printf("%d is done\n", syscall(SYS_gettid));
    fclose(chunk->file_in);
    pthread_exit(0);
}

void* update_balance(void* arg) {
    while (1) {
        // tracks the total sum of all transactions tracked
        // if this is 0, no transactions have been processed since the last update
        // (workers are done)
        double tracker_sum = 0.;
        pthread_mutex_lock(&request_mutex);
        printf("bank %d waiting now, update occurrence %d\n", syscall(SYS_gettid), num_updates);
        while (transactions_processed < REQUEST_THRESHOLD && !final_update) {
            pthread_cond_wait(&updating_balance, &request_mutex);
        }
        // for termination of bank after the last balance update occurs
        // parent thread sends an extra signal to unpause after all worker threads die
        tracker_sum = 0.;
        for (int i = 0; i < num_accounts; i++)
            tracker_sum += account_array[i].transaction_tracter;
        if (tracker_sum == 0) {
            printf("determined no update was needed - bank exiting\n");
            pthread_mutex_unlock(&request_mutex);
            pthread_exit(0);
        }
        for (int i = 0; i < num_accounts; i++) {
            pthread_mutex_lock(&(account_array[i].ac_lock));
            account_array[i].balance += account_array[i].transaction_tracter *
                                        account_array[i].reward_rate;
            // reset transaction tracker to prevent compounding interest
            account_array[i].transaction_tracter = 0.;
            // append is in critical section as well
            FILE* acc = fopen(account_array[i].out_file, "a");
            fprintf(acc, "Current Balance:\t%.2f\n", account_array[i].balance);
            fclose(acc);
            pthread_mutex_unlock(&(account_array[i].ac_lock));
        }
        transactions_processed -= REQUEST_THRESHOLD;
        num_updates++;
        pthread_cond_broadcast(&balance_updated);
        pthread_mutex_unlock(&request_mutex);
    }
    pthread_exit(0);
}

void usage(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [input TXT file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

// used to count total number of transactions in file
// for chunking process
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

    // parent thread buffer for getlines on struct(s) populating
    char* buf = (char*)malloc(buf_size);

    getline(&buf, &buf_size, file_in);
    num_accounts = atoi(buf);
    if (num_accounts == 0) {
        fprintf(stderr, "Error: Invalid number of accounts in input file\n");
        exit(EXIT_FAILURE);
    }
    printf("DuckBank accounts: %d\n", num_accounts);
    account_array = (account*)malloc(sizeof(account) * num_accounts);

    shared = mmap(NULL, sizeof(account) * num_accounts, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

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
    }
    memcpy(shared, account_array, sizeof(account) * num_accounts);

    // user defined signal used to indicate to parent when child is done reading
    // and to child when parent is done writing
    sigset_t sharing;
    int sig;
    sigemptyset(&sharing);
    sigaddset(&sharing, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sharing, NULL);

    pid_t puddles_bank;
    puddles_bank = fork();
    if (puddles_bank < 0) {
        fprintf(stderr, "Error: could not start savings account process\n");
        fclose(file_in);
        free(account_array);
        free(buf);
        exit(EXIT_FAILURE);
    } else if (puddles_bank == 0) { // child process - duck bank
        mkdir("output_savings", 0777);
        printf("PuddlesBank started, waiting for signal\n");
        sigwait(&sharing, &sig);
        printf("PuddlesBank received signal\n");
        // read populated accounts
        memcpy(account_array, shared, sizeof(account) * num_accounts);
        printf("PuddlesBank: finished updating accounts, signaling DuckBank\n");
        kill(getppid(), SIGUSR1);
    } else { // parent process - duck bank
        is_parent++;
        printf("Populating accounts...\n");
        mkdir("output", 0777);
        for (int i = 0; i < num_accounts; i++) {
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
        memcpy(shared, account_array, sizeof(account) * num_accounts);
        printf("DuckBank: finished parsing accounts, signaling PuddlesBank\n");
        for (int i = 0; i < num_accounts; i++) {
            // account number
            memset(shared[i].out_file, '\0', 64);
            snprintf(shared[i].out_file, 63, "./output_savings/%s.txt",
                     shared[i].account_number);
            FILE* acc = fopen(shared[i].out_file, "w");
            fprintf(acc, "account %d:\n", i);
            fclose(acc);
            // initial starting balance is 20% of original balance
            shared[i].balance *= 0.2;
            // flat 2% reward rate
            shared[i].reward_rate = 0.02;
        }
        kill(puddles_bank, SIGUSR1);
        printf("Waiting for PuddlesBank\n");
        sigwait(&sharing, &sig);
        printf("DuckBank received signal\n");
        munmap(shared, sizeof(account) * num_accounts);
        printf("done\n");
    }

    printf("Chunking file %s for processing...", argv[1]);
    file_chunk* file = malloc(sizeof(file_chunk) * NUM_THREADS);
    // count number of transactions to process
    unsigned int transactions = count_lines(file_in);
    unsigned short extras = transactions % NUM_THREADS;
    transactions_start = ftell(file_in);
    // populate file input struct
    for (int i = 0; i < NUM_THREADS; i++) {
        file[i].chunk_lines = CHUNK_SIZE(transactions);
        file[i].local_buf = malloc(LINE_MAX);
        file[i].lines_offset = 0;
        memset(file[i].local_buf, 0, LINE_MAX);
        if (extras > 0) {
            file[i].chunk_lines += 1;
            extras -= 1;
        }

        // summation of all previous offsets
        // chunk 0 starts at transaction 1,
        // chunk 1 12001, chunk 2 24001, etc
        for (int j = 0; j < i; j++)
            file[i].lines_offset += file[j].chunk_lines;
    }
    printf("done\n");

    pthread_t workers[NUM_THREADS];
    pthread_t bank_thread;
    pthread_cond_init(&updating_balance, NULL);
    pthread_cond_init(&balance_updated, NULL);
    pthread_barrier_init(&sync_workers, NULL, NUM_THREADS);
    printf("Processing transactions from %s...\n", argv[1]);
    pthread_create(&bank_thread, NULL, update_balance, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&(workers[i]), NULL, process_transaction, &(file[i]));
    }
    // wait for all workers to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(workers[i], NULL);
    }
    final_update++;
    pthread_mutex_lock(&request_mutex);
    pthread_cond_signal(&updating_balance);
    pthread_mutex_unlock(&request_mutex);
    pthread_join(bank_thread, NULL);
    printf("total updates: %d\n", num_updates);

    pthread_barrier_destroy(&sync_workers);
    printf("done\n");

    if (is_parent) {
        FILE* total_transactions = fopen("output.txt", "w");
        for (int i = 0; i < num_accounts; i++) {
            fprintf(total_transactions, "%d balance:\t%.2f\n\n", i, account_array[i].balance);
        }
        fclose(total_transactions);
    }

    // free allocated chunk members
    for (int i = 0; i < NUM_THREADS; i++) {
        free(file[i].local_buf);
    }
    free(file);
    fclose(file_in);
    free(account_array);
    free(buf);
}