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
#include <errno.h>
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
        printf("Error: could not open current directory for listing\n");
        printf("Check your read permissions for the current directory\n");
        close(fd);
        return;
    }
    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) != 0) {
        if (nread == -1) {
            printf("Error occurred while trying to list current directory");
            break;
        }
        for (bpos = 0; bpos < nread; ) {
            d = (struct linux_dirent*) (buf + bpos);
            printf("%s  ", d->d_name);
            bpos += d->d_reclen;
        }
    }
    printf("\n");
    close(fd);
}

void showCurrentDir() {
    char buf[BUF_SIZE];
    long cwd = syscall(SYS_getcwd, buf, BUF_SIZE);
    if (cwd == -1) {
        printf("Error occurred while getting current working directory\n");
    } else
        printf("%s\n", buf);
}

void makeDir(char *dirName) {
    int fd;
    // create new directory with full permissions
    mode_t mode = 0777;
    fd = open(".", O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        printf("Error: could not open current directory for new creation\n");
        printf("Check your write permissions for the current directory\n");
        close(fd);
        return;
    }
    int made = syscall(SYS_mkdirat, fd, dirName, mode);
    close(fd);
    if (made == -1) {
        printf("Error while creating directory %s: ", dirName);
        switch (errno) {
            case EACCES:
                printf("permission denied\n");
                break;
            case EEXIST:
                printf("directory already exists\n");
                break;
            default:
                printf("unknown error\n");
                break;
        }
    }
}

void changeDir(char *dirName) {
    int changed = syscall(SYS_chdir, dirName);
    if (changed == -1) {
        printf("Error occurred while changing to directory %s: ", dirName);
        switch (errno) {
            case EACCES:
                printf("permission denied\n");
                break;
            case ENOENT:
                printf("directory does not exist\n");
                break;
            case ENOTDIR:
                printf("not a directory\n");
                break;
            default:
                printf("unknown error\n");
                break;
        }
    }
}

void copyFile(char *sourcePath, char *destinationPath) {
    char buf;
    char* create_path = strdup(destinationPath);
    int fd_src, fd_dst;
    fd_src = open(sourcePath, O_RDONLY);
    if (fd_src == -1) {
        printf("Source file %s could not be read: ", sourcePath);
        switch (errno) {
            case EACCES:
                printf("permission denied\n");
                break;
            default:
                printf("unknown error\n");
                break;
        }
        close(fd_src);
        return;
    }
    fd_dst = open(destinationPath, O_RDONLY | O_DIRECTORY);
    if (fd_dst != -1) {
        command_line filename;
        filename = str_filler(sourcePath, "/");
        size_t new_path_size =
            strlen(filename.command_list[filename.num_token - 2]) + 2;
        create_path = realloc(create_path, sizeof(create_path) + new_path_size);
        strcpy(create_path, destinationPath);
        strcat(create_path, "/");
        strcat(create_path, filename.command_list[filename.num_token - 2]);
        free_command_line(&filename);
        close(fd_dst);
    }
    fd_dst = open(create_path, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd_dst == -1) {
        printf("Destination file %s could not be created: ", destinationPath);
        switch (errno) {
            case EACCES:
                printf("permission denied\n");
                break;
            default:
                printf("unknown error\n");
                break;
        }
        close(fd_src);
        close(fd_dst);
        return;
    }
    while (read(fd_src, &buf, 1)) {
        write(fd_dst, &buf, 1);
    }
    close(fd_src);
    close(fd_dst);
    free(create_path);
}

void moveFile(char *sourcePath, char *destinationPath) {
    copyFile(sourcePath, destinationPath);
    unlink(sourcePath);
}

void deleteFile(char *filename) {

}

void displayFile(char *filename) {

}