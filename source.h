#ifndef SOURCE_H
#define SOURCE_H
#define EOF (-1)
#define ERRCHAR (0)
#define INIT_SRC_POS (-2)

struct sourceS
{
    char *buffer;        // Stores user inputted string
    long bufferSize;     // Length of user inputted string
    long curserPosition; // Position of user's cursor
};

char nextChar(struct sourceS *src);
void ungetChar(struct sourceS *src);
char peekChar(struct sourceS *src);
void skipWhiteSpaces(struct sourceS *src);

#endif