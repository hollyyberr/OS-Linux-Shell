#ifndef SCANNER_H
#define SCANNER_H

struct tokenS
{

    struct sourceS *src;
    int textLength;
    char *text;
};

extern struct tokenS eofToken;

struct tokenS *tokenize(struct sourceS *src);

void freeToken(struct tokenS *token);

#endif
