#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "shell.h"
#include "scanner.h"
#include "source.h"

char *tokenBuffer = NULL;
int tokenBufferSize = 0;
int tokenBufferIndex = -1;

/* special token to indicate end of input */
struct tokenS eofToken =
    {
        .textLength = 0,
};

void addToBuffer(char ch)
{
    tokenBuffer[tokenBufferIndex++] = ch;

    if (tokenBufferIndex >= tokenBufferSize)
    {
        char *temp = realloc(tokenBuffer, tokenBufferSize * 2);

        if (!temp)
        {
            errno = ENOMEM;
            return;
        }

        tokenBuffer = temp;
        tokenBufferSize *= 2;
    }
}

struct tokenS *createToken(char *str)
{
    struct tokenS *token = malloc(sizeof(struct tokenS));

    if (!token)
    {
        return NULL;
    }

    memset(token, 0, sizeof(struct tokenS));
    token->textLength = strlen(str);

    char *str2 = malloc(token->textLength + 1);

    if (!str2)
    {
        free(token);
        return NULL;
    }

    strcpy(str2, str);
    token->text = str2;

    return token;
}

void freeToken(struct tokenS *token)
{
    if (token->text)
    {
        free(token->text);
    }
    free(token);
}

struct tokenS *tokenize(struct sourceS *src)
{
    int endLoop = 0;

    if (!src || !src->buffer || !src->bufferSize)
    {
        errno = ENODATA;
        return &eofToken;
    }

    if (!tokenBuffer)
    {
        tokenBufferSize = 1024;
        tokenBuffer = malloc(tokenBufferSize);
        if (!tokenBuffer)
        {
            errno = ENOMEM;
            return &eofToken;
        }
    }

    tokenBufferIndex = 0;
    tokenBuffer[0] = '\0';

    char newch = nextChar(src);
    char newch2;
    int i;

    if (newch == ERRCHAR || newch == EOF)
    {
        return &eofToken;
    }

    do
    {
        switch (newch)
        {
        case '"':
        case '\'':
        case '`':
            addToBuffer(newch);
            i = findClosingQuote(src->buffer + src->cursorPosition);
            if (!i)
            {
                src->cursorPosition = src->bufferSize;
                fprintf(stderr, "error: missing closing quote '%c'\n", newch);
                return &eofToken;
            }
            while (i--)
            {
                addToBuffer(nextChar(src));
            }
            break;

        case '\\':
            newch2 = nextChar(src);
            if (newch2 == '\n')
            {
                break;
            }

            addToBuffer(newch);

            if (newch2 > 0)
            {
                addToBuffer(newch2);
            }
            break;

        case '$':
            addToBuffer(newch);
            newch = peekChar(src);

            if (newch == '{' || newch == '(')
            {
                i = findClosingBrace(src->buffer + src->cursorPosition + 1);
                if (!i)
                {
                    src->cursorPosition = src->bufferSize;
                    fprintf(stderr, "error: missing closing brace '%c'\n", newch);
                    return &eofToken;
                }

                while (i--)
                {
                    addToBuffer(nextChar(src));
                }
            }
            else if (isalnum(newch) || newch == '*' || newch == '@' || newch == '#' ||
                                       newch == '!' || newch == '?' || newch == '$')
            {
                addToBuffer(nextChar(src));
            }
            break;
        case ' ':
        case '\t':
            if (tokenBufferIndex > 0)
            {
                endLoop = 1;
            }
            break;

        case '\n':
            if (tokenBufferIndex > 0)
            {
                ungetChar(src);
            }
            else
            {
                addToBuffer(newch);
            }
            endLoop = 1;
            break;

        default:
            addToBuffer(newch);
            break;
        }

        if (endLoop)
        {
            break;
        }

    } while ((newch = nextChar(src)) != EOF);

    if (tokenBufferIndex == 0)
    {
        return &eofToken;
    }

    if (tokenBufferIndex >= tokenBufferSize)
    {
        tokenBufferIndex--;
    }
    tokenBuffer[tokenBufferIndex] = '\0';

    struct tokenS *token = createToken(tokenBuffer);
    if (!token)
    {
        fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
        return &eofToken;
    }

    token->src = src;
    return token;
}