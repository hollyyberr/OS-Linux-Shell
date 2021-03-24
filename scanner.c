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

struct tokenS eofToken = {
    .textLength = 0,
};

// Adds character to command buffer
void addToBuffer(char c) {
    
    tokenBuffer[tokenBufferIndex++] = c;

    if(tokenBufferIndex >= tokenBufferSize) {

        char *temp = realloc(tokenBuffer, tokenBufferSize*2);

        if(!temp) {
            
            errno = ENOMEM;
            return;

        }

        tokenBuffer = temp;
        tokenBufferSize *= 2;
    }
}

// Creates token from string entered
struct tokenS *createToken(char *str) {

    struct tokenS *token = malloc(sizeof(struct tokenS));

    if(!token) {
        return NULL;
    }

    memset(token, 0, sizeof(struct tokenS));
    token->textLength = strlen(str);
    char *nstr = malloc(token->textLength + 1);

    if(!nstr) {
        free(token);
        return NULL;
    }

    strcpy(nstr, str);
    token->text = nstr;
    return token;

}

// Frees Token
void freeToken(struct tokenS *token) {
    if(token->text) {
        free(token->text);
    }
    free(token);
}

// Tokenize
struct tokenS *tokenize(struct sourceS *src) {
    int stopLoop = 0;

    if(!src || !src->buffer || !src->bufferSize) {
        errno = ENODATA;
        return &eofToken;
    }

    if(!tokenBuffer) {
        tokenBufferSize = 1024;
        tokenBuffer = malloc(tokenBufferSize);
        if(!tokenBuffer) {
            errno = ENOMEM;
            return &eofToken;
        }
    }

    tokenBufferIndex = 0;
    tokenBuffer[0] = '\0';
    char noChar = nextChar(src);

    if(noChar == ERRCHAR || noChar == EOF) {
        return &eofToken;
    }

    do {

        switch(noChar) {
            case ' ':
            case '\t':
                if(tokenBufferIndex > 0) {
                    stopLoop = 1;
                }
                break;
            case '\n':
                if(tokenBufferIndex > 0) {
                    ungetChar(src);
                }
                else {
                    addToBuffer(noChar);
                }
                stopLoop = 1;
                break;
            default:
                addToBuffer(noChar);
                break;
        }
        if(stopLoop) {
            break;
        }
    } while((noChar = nextChar(src)) != EOF);

    if(tokenBufferIndex == 0) {
        return &eofToken;
    }

    if(tokenBufferIndex >= tokenBufferSize) {
        tokenBufferIndex --;
    }

    tokenBuffer[tokenBufferIndex] = '\0';

    struct tokenS *token = createToken(tokenBuffer);
    if(!token) {
        fprintf(stderr, "error: could not allocate buffer: %s\n", strerror(errno));
        return &eofToken;
    }

    token->src = src;
    return token;
}