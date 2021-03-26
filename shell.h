#ifndef SHELL_H
#define SHELL_H

#include "source.h"

// Source code for shell activities
void printPrompt1(void);
void printPrompt2(void);

char *readCommand(void);
int parseAndExecute(struct sourceS *src);

#endif