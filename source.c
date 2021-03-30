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

    long position = src->cursorPosition;

    if (position == INIT_SRC_POS)
    {
        position++;
    }
    position++;

    if (position >= src->bufferSize)
    {
        return EOF;
    }

    return src->buffer[position];
}

void skipWhiteSpaces(struct sourceS *src)
{
    char ch;

    if (!src || !src->buffer)
    {
        return;
    }

    while (((ch = peekChar(src)) != EOF) && (ch == ' ' || ch == '\t'))
    {
        nextChar(src);
    }
}