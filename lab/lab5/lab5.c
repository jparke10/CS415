#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define _GNU_SOURCE

void signaler(pid_t* pid_ary, int size, int signal);

int main(int argc, char* argv[])
{
	
	// initialization of pid etc. like lab 4
	const char* command = "./iobound";
	char* const command_argv[] = {"./iobound", "-seconds", "5", NULL};
	const size_t n = atoi(argv[1]);
	pid_t pid;
	pid_t* pid_array = malloc(sizeof(pid_t) * n);

	// initialize sigset
	sigset_t sigset;
	int sig;

	// create an empty sigset_t
	sigemptyset(&sigset);
	// use sigaddset() to add the SIGUSR1 signal to the set
	sigaddset(&sigset, SIGUSR1);
	// use sigprocmask() to add the signal set in the sigset for blocking
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	for(int i = 0; i < n; i++)
	{
		pid = fork();
		if (pid > 0) {
			// same fork as in lab 4
			pid_array[i] = pid;
		}

		if(pid == 0)
		{
			// print: Child Process: <pid> - Waiting for SIGUSR1…
			printf("Child Process: %d - Waiting for SIGUSR1...\n", getpid());
			// wait for the signal
			sigwait(&sigset, &sig);
			// print: Child Process: <pid> - Received signal: SIGUSR1 - Calling exec().
			printf("Child Process: %d - Received signal: SIGUSR1 - Calling exec().\n", getpid());
			// call execvp with ./iobound like in lab 4
			execvp(command, command_argv);
		}
		if (pid < 0) {
			// fork failed error
			fprintf(stderr, "fork failed\n");
			exit(EXIT_FAILURE);
		}
	}
	
	// send SIGUSR1 
	signaler(pid_array, n, SIGUSR1);
	// send SIGSTOP 
	signaler(pid_array, n, SIGSTOP);
	// send SIGCONT
	signaler(pid_array, n, SIGCONT);
	// send SIGINT
	signaler(pid_array, n, SIGINT);
	free(pid_array);
	return 0;
}

void signaler(pid_t* pid_ary, int size, int signal)
{
	// sleep for three seconds
	sleep(3);
	for(int i = 0; i < size; i++)
	{
		// print: Parent process: <pid> - Sending signal: <signal> to child process: <pid>
		printf("Parent process: %d - Sending signal %s to child process: %d\n",
				getpid(), strsignal(signal), pid_ary[i]);
		// send the signal
		kill(pid_ary[i], signal);
	}
}



