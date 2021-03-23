#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "shell.h"

int main (int argc, char **argv) {

    char *command;
    do {
        printPrompt1();
        command = read_cmd();

        if(!command) {
            exit(EXIT_SUCCESS);
        }

        if(command[0] == '\0' || strcmp(command, "\n") == 0) {
            free(command);
            continue;
        }

        if(strcmp(command, "exit\n") == 0) {
            free(command);
            break;
        }

        printf("%s\n", command);
        free(command);

    }

    while(1);
    exit(EXIT_SUCCESS);
}

char *readCommand(void) {

    char buffer[1024];
    char *pointer = NULL;
    char pointerLen = 0;

    while(fgets(buffer, 1024, stdin)) {
        
        int bufferLen = strlen(buf);

        if(!pointer) {
            pointer = malloc(bufferLen + 1);
        }
        else {
            char *pointer2 = realloc(pointer, pointerLen + bufferLen + 1)

            if(pointer2) {
                pointer = pointer2;
            }
            else {
                free(pointer);
                pointer = NULL;
            }
        }

        if(!pointer) {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }

        strcpy(pointer + pointerLen, buffer);

        if(buffer[bufferLen-1] == '\n') {

            if(bufferLen == 1 || buffer[bufferLen - 2] != '\\') {
                return pointer;
            }

            pointer[pointerLen + bufferLen - 2] = '\0';
            bufferLen -= 2;
            printPrompt2();

        }

        pointerLen += bufferLen;
    }

    return pointer;
}


























