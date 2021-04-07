#ifndef SOURCE_H
#define SOURCE_H
#define EOF (-1)
#define ERRCHAR (0)
#define INIT_SRC_POS (-2)

struct sourceS
{
    char *buffer;        // Stores user inputted string
    long bufferSize;     // Length of user inputted string
    long cursorPosition; // Position of user's cursor
};

char nextChar(struct sourceS *s);
void ungetChar(struct sourceS *s);
char peekChar(struct sourceS *s);
void skipWhiteSpaces(struct sourceS *s);

#endif