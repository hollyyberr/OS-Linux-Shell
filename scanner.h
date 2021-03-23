#ifndef SCANNER_H
#define SCANNER_H

struct tokenS
{

    struct sourceS *src; // Where input is coming from
    int textLength;      // Length of user input
    char *text;          // Content of user input
};

extern struct tokenS eofToken; // End of user input token

struct tokenS *tokenize(struct sourceS *src); // Gets next piece of user input

void freeToken(struct tokenS *token);

#endif
