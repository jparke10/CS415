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

int copy_success;

// errno switcher for handling most syscall errors
void handleError() {
    switch (errno) {
        case EEXIST:
            printf("already exists\n");
            break;
        case ENOENT:
            printf("does not exist\n");
            break;
        case ENOTDIR:
            printf("not a directory\n");
            break;
        case EACCES:
            printf("permission denied\n");
            break;
        case EISDIR:
            printf("is a directory\n");
            break;
        default:
            printf("unknown error\n");
            break;
    }
}

void listDir() {
    int fd, bpos, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;

    fd = open(".", O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        printf("Error: could not open current directory for listing\n");
        return;
    }
    // getdents64 creates dirents in buf, which contain directory names
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
    int cwd = syscall(SYS_getcwd, buf, BUF_SIZE);
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
        printf("Error: could not open current directory for creation\n");
        return;
    }
    int made = syscall(SYS_mkdirat, fd, dirName, mode);
    close(fd);
    if (made == -1) {
        printf("Error while creating directory %s: ", dirName);
        handleError();
    }
}

void changeDir(char *dirName) {
    int changed = syscall(SYS_chdir, dirName);
    if (changed == -1) {
        printf("Error occurred while changing to directory %s: ", dirName);
        handleError();
    }
}

void copyFile(char *sourcePath, char *destinationPath) {
    copy_success = -1;
    char buf;
    // duplicate destinationPath for possible reconstruction later
    char* create_path = strdup(destinationPath);
    int fd_src, fd_dst;
    fd_src = open(sourcePath, O_RDONLY);
    if (fd_src == -1) {
        printf("Source file %s could not be read: ", sourcePath);
        handleError();
        return;
    }
    // try to first open destinationPath as directory
    fd_dst = open(destinationPath, O_RDONLY | O_DIRECTORY);
    // if it's a directory, construct new path 
    if (fd_dst != -1) {
        close(fd_dst);
        command_line filename;
        filename = str_filler(sourcePath, "/");
        /* +2 to account for / and \0 characters
           [filename.num_token - 2] is the last non-NULL token in sourcePath
           aka, the source file's actual name */
        size_t new_path_size =
            strlen(filename.command_list[filename.num_token - 2]) + 2;
        create_path = realloc(create_path, strlen(create_path) + new_path_size);
        strcpy(create_path, destinationPath);
        strcat(create_path, "/");
        strcat(create_path, filename.command_list[filename.num_token - 2]);
        free_command_line(&filename);
    }
    // dst file will have generic 644 permissions
    // O_TRUNC zeroes out destination file if it already exists
    fd_dst = open(create_path, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd_dst == -1) {
        printf("Destination file %s could not be created: ", destinationPath);
        handleError();
        return;
    }
    while (read(fd_src, &buf, 1)) {
        write(fd_dst, &buf, 1);
    }
    close(fd_src);
    close(fd_dst);
    free(create_path);
    copy_success = 0;
}

void moveFile(char *sourcePath, char *destinationPath) {
    copyFile(sourcePath, destinationPath);
    if (copy_success == 0)
        unlink(sourcePath);
}

void deleteFile(char *filename) {
    int deleted = unlink(filename);
    if (deleted == -1) {
        printf("Error while removing file %s: ", filename);
        handleError();
    }
}

void displayFile(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("File %s could not be opened: ", filename);
        handleError();
        return;
    }
    off_t file_length = syscall(SYS_lseek, fd, 0, SEEK_END);
    if (file_length == -1) {
        printf("Error while trying to get length of file %s\n", filename);
        close(fd);
        return;
    }
    syscall(SYS_lseek, fd, 0, SEEK_SET);
    char file[file_length];
    ssize_t nread = read(fd, file, sizeof(file));
    write(STDOUT_FILENO, file, nread);
    write(STDOUT_FILENO, "\n", 1);
}