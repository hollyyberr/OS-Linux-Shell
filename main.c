#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "shell.h"
#include "source.h"
#include "parser.h"
#include "executor.h"

int main(int argc, char **argv)
{
    char *command;

    do
    {
        printPrompt1();

        command = readCommmand();

        if (!command)
        {
            exit(EXIT_SUCCESS);
        }

        if (command[0] == '\0' || strcmp(command, "\n") == 0)
        {
            free(command);
            continue;
        }

        if (strcmp(command, "exit\n") == 0)
        {
            free(command);
            break;
        }

        struct sourceS src;
        src.buffer = command;
        src.bufferSize = strlen(command);
        src.cursorPosition = INIT_SRC_POS;
        parseAndExecute(&src);

        free(command);

    } while (1);

    exit(EXIT_SUCCESS);
}

char *readCommand(void)
{
    char buffer[1024];
    char *pointer = NULL;
    char pointerLength = 0;

    while (fgets(buffer, 1024, stdin))
    {
        int bufferLen = strlen(buffer);

        if (!pointer)
        {
            pointer = malloc(bufferLen + 1);
        }
        else
        {
            char *pointer2 = realloc(pointer, pointerLength + bufferLen + 1);

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
        {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }

        strcpy(pointer + pointerLength, buffer);

        if (buf[bufferLen - 1] == '\n')
        {
            if (bufferLen == 1 || buffer[bufferLen - 2] != '\\')
            {
                return pointer;
            }

            ptr[pointerLength + bufferLen - 2] = '\0';
            bufferLen -= 2;
            printPrompt2();
        }

        pointerLength += bufferLen;
    }

    return pointer;
}

int parseAndExecute(struct sourceS *src)
{
    skipWhiteSpaces(src);

    struct tokenS *token = tokenize(src);

    if (token == &eofToken)
    {
        return 0;
    }

    while (token && token != &eofToken)
    {
        struct nodeS *command = parseSimpleCommand(token);

        if (!command)
        {
            break;
        }

        doSimpleCommand(command);
        freeNodeTree(command);
        token = tokenize(src);
    }

    return 1;
}
