#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../shell.h"
#include "../node.h"
#include "../parser.h"
#include "symtab.h"

struct symtabStackS symtabStack;
int symtabLevel;

void initSymtab(void)
// Initializes new symbol tab for table
{
    symtabStack.symtabCount = 1;
    symtabLevel = 0;

    struct symtabS *globalSymtab = malloc(sizeof(struct symtabS));

    if (!globalSymtab)
    {
        fprintf(stderr, "fatal error: Could not create global symbol table due to lack of memory\n");
        exit(EXIT_FAILURE);
    }

    memset(globalSymtab, 0, sizeof(struct symtabS));
    symtabStack.globalSymtab = globalSymtab;
    symtabStack.localSymtab = globalSymtab;
    symtabStack.symtabList[0] = globalSymtab;
    globalSymtab->level = 0;
}

struct symtabS *newSymtab(int level)
// Used to create a new symbol table
{
    struct symtabS *symtab = malloc(sizeof(struct symtabS));

    if (!symtab)
    {
        fprintf(stderr, "fatal error: Could not create new symbol table due to lack of memory\n");
        exit(EXIT_FAILURE);
    }

    memset(symtab, 0, sizeof(struct symtabS));
    symtab->level = level;
    return symtab;
}

void freeSymtab(struct symtabS *symtab)
// Used to clear memory of symbol tables that are no longer in use
{
    if (symtab == NULL)
    {
        return;
    }

    struct symtabEntryS *entry = symtab->first;

    while (entry)
    {
        if (entry->name)
        {
            free(entry->name);
        }

        if (entry->val)
        {
            free(entry->val);
        }

        if (entry->funcBody)
        {
            freeNodeTree(entry->funcBody);
        }

        struct symtabEntryS *next = entry->next;
        free(entry);
        entry = next;
    }

    free(symtab);
}