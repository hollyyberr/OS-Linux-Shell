#include <errno.h>
#include "shell.h"
#include "source.h"

void ungetChar(struct sourceS *s)
{
    if (s->cursorPosition < 0)
    {
        return;
    }
    s->cursorPosition--;
}

char nextChar(struct sourceS *s)
{
    if (!s || !s->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }
    if (s->cursorPosition == INIT_SRC_POS)
    {
        s->cursorPosition = -1;
    }
    if (++s->cursorPosition >= s->bufferSize)
    {
        s->cursorPosition = s->bufferSize;
        return EOF;
    }
    return s->buffer[s->cursorPosition];
}

char peekChar(struct sourceS *s)
{
    if (!s || !s->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }
    long position = s->cursorPosition;

    if (position == INIT_SRC_POS)
    {
        position++;
    }
    position++;

    if (position >= s->bufferSize)
    {
        return EOF;
    }
    return s->buffer[position];
}

void skipWhiteSpaces(struct sourceS *s)
{
    char a;

    if (!s || !s->buffer)
    {
        return;
    }

    while (((a = peekChar(s)) != EOF) && (a == ' ' || a == '\t'))
    {
        nextChar(s);
    }
}