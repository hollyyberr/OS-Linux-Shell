#include <stdlib.h>
#include <string.h>
#include "shell.h"

// Search input string for passed character
char *strchrAny(char *str, char *cs)
{
    if (!str || !cs)
    {
        return NULL;
    }
    char *a = str;
    while (*a)
    {
        char *b = cs;
        while (*b)
        {
            if (*a == *b)
            {
                return a;
            }
            b++;
        }
        a++;
    }
    return NULL;
}

// Returns reformatted string
char *quoteVal(char *v, int addQuotes)
{
    char *result = NULL;
    size_t length;
    // Empty str
    if (!v || !*v)
    {
        length = addQuotes ? 3 : 1;
        result = malloc(length);
        if (!result)
        {
            return NULL;
        }
        strcpy(result, addQuotes ? "\"\"" : "");
        return result;
    }
    // Num of quotes needed
    length = 0;
    char *c = v, *a;
    while (*c)
    {
        switch (*c)
        {
        case '\\':
        case '`':
        case '$':
        case '"':
            length++;
            break;
        }
        c++;
    }
    length += strlen(v);
    // Quotes for brackets
    if (addQuotes)
    {
        length += 2;
    }
    // Allocate mem for string
    result = malloc(length + 1);
    if (!result)
    {
        return NULL;
    }
    a = result;
    if (addQuotes)
    {
        *a++ = '"';
    }
    c = v;
    while (*c)
    {
        switch (*c)
        {
        case '\\':
        case '`':
        case '$':
        case '"':
            *a++ = '\\';
            *a++ = *c++;
            break;

        default:
            *a++ = *c++;
            break;
        }
    }
    if (addQuotes)
    {
        *a++ = '"';
    }
    *a = '\0';
    return result;
}

int checkBufferBounds(int *cnt, int *length, char ***buffer)
{
    if (*cnt >= *length)
    {
        if (!(*buffer))
        {
            *buffer = malloc(32 * sizeof(char **));
            if (!(*buffer))
            {
                return 0;
            }
            *length = 32;
        }
        else
        {
            int newLength = (*length) * 2;
            char **c2 = realloc(*buffer, newLength * sizeof(char **));
            if (!c2)
            {
                return 0;
            }
            *buffer = c2;
            *length = newLength;
        }
    }
    return 1;
}

// Free memory
void freeBuffer(int length, char **buffer)
{
    if (!length)
    {
        return;
    }

    while (length--)
    {
        free(buffer[length]);
    }
    free(buffer);
}