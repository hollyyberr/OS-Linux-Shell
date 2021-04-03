#ifndef SHELL_H
#define SHELL_H

// Source code for shell activities
void printPrompt1(void);
void printPrompt2(void);

char *readCommand(void);
void initsh(void);

#include "source.h"
int parseAndExecute(struct sourceS *src);

// Shell built in utils
int dump(int argc, char **argv);

// Struct for built in utils
struct builtinS
{   
    // Util name
    char *name;

    // Func to call and execute util
    int (*func)(int argc, char **argv);
};

// List of built in utils
extern struct builtinS builtins[];

// Util count
extern int builtinsCount;

#endif