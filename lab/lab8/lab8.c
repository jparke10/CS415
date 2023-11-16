#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

#define MAX_THREADS 50

pthread_t thread_id[MAX_THREADS];    

void * Print_thread(void* i)
{
    // TODO: Allocate the mutex lock
    pthread_mutex_lock(&mutex1);
    // TODO: print "First Print: " and the int i
    printf("First Print: %d\n", *(int*)i);
    // TODO: sleep for one second
    sleep(1);
    // TODO: print "Second Print:" and the int i
    printf("Second Print: %d\n", *(int*)i);
    // TODO: Release the mutex lock
    pthread_mutex_unlock(&mutex1);
    // TODO: You may also comment out the lock and unlock and see how the output changes
    pthread_exit(0);
}

int main(int argc, char * argv[])
{
    int rc, i, n;

    if(argc < 2) 
    {
        printf("Please add the number of threads to the command line\n");
        exit(1); 
    }
    n = atoi(argv[1]);
    if(n > MAX_THREADS) n = MAX_THREADS;

    // TODO: Create n threads, each calling Print_thread
    // You may add print statements here as well if you want
    for (i = 0; i < n; i++) {
        pthread_create(&(thread_id[i]), NULL, Print_thread, &i);
    }
    for (i = 0; i < n; i++) {
        pthread_join(thread_id[i], 0);
    }
}