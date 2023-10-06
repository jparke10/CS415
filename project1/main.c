#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"
#include "command.h"

void shellFileMode(char* filename) {

}

void shellInteractiveMode() {

}

int main(int argc, char** argv) {
    (void) argv;
    // checking for command line argument
    switch (argc) {
        case 1:
            shellInteractiveMode();
            exit(EXIT_SUCCESS);
        case 3:
            if (strcmp(argv[1], "-f") == 0) {
                shellFileMode(argv[2]);
                exit(EXIT_SUCCESS);
            }
    }
    fprintf(stderr, "Usage: %s [-f] [FILE]\n", argv[0]);
    exit(EXIT_FAILURE);
}