#include<stdio.h>
#include <sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

void script_print (pid_t* pid_ary, int size);

int main(int argc,char*argv[])
{
	if (argc != 2)
	{
		printf ("Wrong number of arguments\n");
		exit (0);
	}

	/*
	*	TODO
	*	#1	declare child process pool
	*	#2 	spawn n new processes
	*		first create the argument needed for the processes
	*		for example "./iobound -seconds 10"
	*	#3	call script_print
	*	#4	wait for children processes to finish
	*	#5	free any dynamic memory
	*/
	const char* command = "./iobound";
	char* const command_argv[] = {"./iobound", "-seconds", "5", NULL};

	const size_t processes = atoi(argv[1]);
	pid_t pid, wpid;
	pid_t* pid_array = malloc(sizeof(pid_t) * processes);
	size_t i = 0;
	while (i < processes) {
		pid = fork();
		if (pid < 0) {
			fprintf(stderr, "error in fork\n");
			exit(EXIT_FAILURE);
		} else if (pid == 0) {
			execvp(command, command_argv);
		} else {
			pid_array[i++] = pid;
		}
	}
	script_print(pid_array, processes);
	while ((wpid = wait(NULL)) > 0);
	free(pid_array);
	return 0;
}


void script_print (pid_t* pid_ary, int size)
{
	FILE* fout;
	fout = fopen ("top_script.sh", "w");
	fprintf(fout, "#!/bin/sh\ntop");
	for (int i = 0; i < size; i++)
	{
		fprintf(fout, " -p %d", (int)(pid_ary[i]));
	}
	fprintf(fout, "\n");
	fclose (fout);

	char* top_arg[] = {"bash", "top_script.sh", NULL};
	pid_t top_pid;

	top_pid = fork();
	{
		if (top_pid == 0)
		{
			if(execvp(top_arg[0], top_arg) == -1)
			{
				perror ("top command: ");
			}
			exit(0);
		}
	}
}


