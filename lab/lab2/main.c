#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"

#define _GNU_SOURCE

int main(int argc, char** argv)
{
	(void) argv;
	unsigned short close_stdout = 0;
	//checking for command line argument
	if (argc != 1)
	{
		printf ("Usage: [path to executable from directory to list]\n");
	}

	//declear line_buffer
	size_t len = 128;
	char* line_buf = malloc(len);
    printf(">>> ");
	//loop until terminated by user
	while (getline (&line_buf, &len, stdin) != -1)
	{
        if (strcmp(line_buf, "lfcat\n") == 0) {
            lfcat();
			close_stdout = 1;
            printf(">>> ");
        } else if (strcmp(line_buf, "exit\n") == 0)
            break;
        else {
            printf("Error: Unrecognized command!\n");
            printf(">>> ");
        }
	}
	// if lfcat was executed, close re-opened stdout to prevent leaks
	if (close_stdout)
		fclose(stdout);
    free(line_buf);
}