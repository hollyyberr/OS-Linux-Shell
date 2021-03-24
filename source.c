#include <errno.h>
#include "shell.h"
#include "source.h"

void ungetChar(struct sourceS *src)
{
    if (src->cursorPosition < 0)
    {
        return;
    }

    src->cursorPosition--;
}

char nextChar(struct sourceS *src)
{
    if (!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }

    char char1 = 0;
    if (src->cursorPosition == INIT_SRC_POS)
    {
        src->cursorPosition = -1;
    }
    else
    {
        char1 = src->buffer[src->cursorPosition];
    }

    if (++src->cursorPosition >= src->bufferSize)
    {
        src->cursorPosition = src->bufferSize;
        return EOF;
    }

    return src->buffer[src->cursorPosition];
}

char peekChar(struct sourceS *src)
{
    if (!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }

    long thisPos = src->cursorPosition;

    if (thisPos == INIT_SRC_POS)
    {
        thisPos++;
    }
    thisPos++;

    if (thisPos >= src->bufferSize)
    {
        return EOF;
    }

    return src->buffer[thisPos];
}

void skipWhiteSpaces(struct sourceS *src)
{
    char thisChar;

    if (!src || !src->buffer)
    {
        return;
    }

    while (((thisChar = peekChar(src)) != EOF) && (thisChar == ' ' || thisChar == '\t'))
    {
        nextChar(src);
    }
}