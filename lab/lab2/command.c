#include "command.h"

void lfcat()
{
/* High level functionality you need to implement: */
	/* Get the current directory with getcwd() */
	char* path_buf = malloc(PATH_MAX);
	char* pathname = getcwd(path_buf, PATH_MAX);
	/* Open the dir using opendir() */
	DIR* dp;
	dp = opendir(pathname);
	/* use a while loop to read the dir with readdir()*/
	struct dirent* dirp;
	// printf("Listing all files in the current directory\n");
	/* redirect stdout to file output */
	freopen("output.txt", "w", stdout);
	while ((dirp = readdir(dp)) != NULL) {
		/* You can debug by printing out the filenames here */
		// if (dirp->d_type == DT_REG)
		//     printf("File: %s\n", dirp->d_name);	
		/* Option: use an if statement to skip any names that are not readable files (e.g. ".", "..", "main.c", "lab2.exe", "output.txt" */
		if (dirp->d_type == DT_REG) {
			fprintf(stdout, "File: %s\n", dirp->d_name);
			char* line_buf = NULL;
			size_t n = 0;
			/* Open the file */
			FILE* file_in = fopen(dirp->d_name, "r");
			if (file_in == NULL)
				exit(EXIT_FAILURE);
			/* Read in each line using getline() */
			while (getline(&line_buf, &n, file_in) != EOF) {
				/* Write the line to stdout */
				write(STDOUT_FILENO, "test\n", 5);
			}
			/* write 80 "-" characters to stdout */
			char* dash = malloc(82);
			memset(dash, '-', 80);
			memset(dash + 80, '\n', 1);
			write(STDOUT_FILENO, dash, 81);
			free(dash);
			/* close the read file and free/null assign your line buffer */
			free(line_buf);
			fclose(file_in);
		}
	}
	fclose(stdout);
	stdout = fdopen(0, "w");
	/*close the directory you were reading from using closedir() */
	closedir(dp);

	free(path_buf);
}
