#include "main.h"

int main(int argc, char** argv) {
    char *input;
    size_t n = 0;

    printf(">>> ");
    getline(&input, &n, stdin);
    printf("%s\n", input);
    
    free(input);
    return 0;
}