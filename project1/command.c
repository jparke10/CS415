#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define BUF_SIZE 1024

struct linux_dirent {
    unsigned long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};

void listDir() {
    int fd;
    char buf[BUF_SIZE];
    long nread;
    struct linux_dirent *d;

    fd = open(".", O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        exit(EXIT_FAILURE);
    
    for (;;) {
        nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
        if (nread == -1)
            exit(EXIT_FAILURE);
        if (nread == 0)
            break;

        for (size_t bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            printf("%s ", d->d_name);
        }
        printf("\n");
    }

}

void showCurrentDir() {

}

void makeDir(char* dirName) {

}

void changeDir(char* dirName) {

}

void copyFile(char* sourcePath, char* destinationPath) {

}

void moveFile(char* sourcePath, char* destinationPath) {

}

void deleteFile(char* filename) {

}

void displayFile(char* filename) {

}