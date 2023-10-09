#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <limits.h>
#include <string.h>
#include "string_parser.h"
#include "command.h"

#define BUF_SIZE 1024

struct linux_dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};

void listDir() {
    int fd, bpos, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;

    fd = open(".", O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        fprintf(stderr, "Error: could not open current directory for listing\n");
        fprintf(stderr, "Check your read permissions for the current directory\n");
        exit(EXIT_FAILURE);
    }
    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) != 0) {
        if (nread == -1) {
            fprintf(stderr, "Error occurred while trying to list current directory");
            exit(EXIT_FAILURE);
        }
        for (bpos = 0; bpos < nread; ) {
            d = (struct linux_dirent*) (buf + bpos);
            printf("%s  ", d->d_name);
            bpos += d->d_reclen;
        }
    }
    printf("\n");
}

void showCurrentDir() {
    char buf[BUF_SIZE];
    long cwd = syscall(SYS_getcwd, buf, BUF_SIZE);
    if (cwd == -1) {
        fprintf(stderr,
                "Error occurred while getting current working directory\n");
        exit(EXIT_FAILURE);
    } else
        printf("%s\n", buf);
}

void makeDir(char *dirName) {
    int fd, made;
    // create new directory with full permissions
    mode_t mode = 0777;
    fd = open(".", O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        fprintf(stderr, "Error: could not open current directory for new creation\n");
        fprintf(stderr, "Check your read permissions for the current directory\n");
        exit(EXIT_FAILURE);
    }
    made = syscall(SYS_mkdirat, fd, dirName, mode);
    if (made == -1) {
        fprintf(stderr,
                "Error occurred while trying to make directory %s\n",
                dirName);
        exit(EXIT_FAILURE);
    }
}

void changeDir(char *dirName) {

}

void copyFile(char *sourcePath, char *destinationPath) {

}

void moveFile(char *sourcePath, char *destinationPath) {

}

void deleteFile(char *filename) {

}

void displayFile(char *filename) {

}