#include <stdio.h>
#include "shell.h"
#include "symtab/symtab.h"

// Prints initial prompt to users shell
void printPrompt1(void)
{
    struct symtabEntryS *ent = getSymtabEntry("PS1");

    if(ent && ent->val)
    {
        fprintf(stderr, "%s", ent->val);
    }
    else
    {
        fprintf(stderr, "$ ");
    }
}

// Prints prompt for multi-line commands when needed
void printPrompt2(void)
{
    struct symtabEntryS *ent = getSymtabEntry("PS2");

    if(ent && ent->val)
    {
        fprintf(stderr, "%s", ent->val);
    }
    else
    {
        fprintf(stderr, "> ");
    }
}