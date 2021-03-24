#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "shell.h"

int main(int argc, char **argv)
{
    bool end = false;
    
    char *command;

    while(!end) {
        // Prints initial prompt to users shell
        printPrompt1();
        // Reads command that user has entered
        command = readCommand();

        if (!command)
        { // If user has not entered a command, shell exits
            exit(EXIT_SUCCESS);
        }

        if (command[0] == '\0' || strcmp(command, "\n") == 0)
        { // Returns information that the user has entered
            free(command);
            continue;
        }

        if (strcmp(command, "exit\n") == 0)
        { // If the user has entered 'exit' shell breaks
            free(command);
            exit(EXIT_SUCCESS);
            //break;
        }

        printf("%s\n", command);
        free(command);
    }

    //while (1);
    //exit(EXIT_SUCCESS);
}

char *readCommand(void)
{ // Used to read the command that the user has entered

    char buffer[1024]; // Creates storage to hold user entered data
    char *pointer = NULL;
    char pointerLen = 0;

    while (fgets(buffer, 1024, stdin))
    {

        int bufferLen = strlen(buffer);

        if (!pointer)
        { // Fills pointer with user entered data
            pointer = malloc(bufferLen + 1);
        }
        else
        { // Multiple sections of the user entered command are stored
            char *pointer2 = realloc(pointer, pointerLen + bufferLen + 1);

            if (pointer2)
            {
                pointer = pointer2;
            }
            else
            {
                free(pointer);
                pointer = NULL;
            }
        }

        if (!pointer)
        { // Returns error due to memory issue
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }

        // Input is copied to buffer
        strcpy(pointer + pointerLen, buffer);

        if (buffer[bufferLen - 1] == '\n')
        { // Checks if user input is escaped with '\n', until there are no more '\n'

            if (bufferLen == 1 || buffer[bufferLen - 2] != '\\')
            {
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
