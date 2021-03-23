#include <stdio.h>
#include "shell.h"

void printPrompt1(void) {
    fprintf(stderr, "$ ");
}

void printPrompt2(void) {
    fprintf(stderr, "> ");
}