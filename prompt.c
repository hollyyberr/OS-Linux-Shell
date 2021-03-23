#include <stdio.h>
#include "shell.h"

// Prints initial prompt to users shell
void printPrompt1(void)
{
    fprintf(stderr, "$ ");
}

// Prints prompt for multi-line commands when needed
void printPrompt2(void)
{
    fprintf(stderr, "> ");
}