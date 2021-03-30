#ifndef SHELL_H
#define SHELL_H

// Source code for shell activities
void printPrompt1(void);
void printPrompt2(void);

char *readCommand(void);

#include "source.h"
int parseAndExecute(struct sourceS *src);

#endif