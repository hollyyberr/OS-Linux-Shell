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

void dumpLocalSymtab(void)
// Used to debug the shell
{
    struct symtabS *symtab = symtabStack.localSymtab;
    int i = 0;
    int ind = symtab->level * 4;

    fprintf(stderr, "%*sSymbol table [Level %d]:\r\n", ind, " ", symtab->level);
    fprintf(stderr, "%*s===========================\r\n", ind, " ");
    fprintf(stderr, "%*s  No               Symbol                    Val\r\n", ind, " ");
    fprintf(stderr, "%*s------ -------------------------------- ------------\r\n", ind, " ");

    struct symtabEntryS *ent = symtab->first;

    while (ent)
    {
        fprintf(stderr, "%*s[%04d] %-32s '%s'\r\n", ind, " ",
                i++, ent->name, ent->val);
        ent = ent->next;
    }

    fprintf(stderr, "%*s------ -------------------------------- ------------\r\n", ind, " ");
}

struct symtabEntryS *addToSymtab(char *symb)
// Used to add new entry to symbol table
{
    if (!symb || symb[0] == '\0')
    {
        return NULL;
    }

    struct symtabS *ns = symtabStack.localSymtab;
    struct symtabEntryS *ent = NULL;

    if ((ent = do_lookup(symb, ns)))
    {
        return ent;
    }

    ent = malloc(sizeof(struct symtabEntryS));

    if (!ent)
    {
        fprintf(stderr, "fatal error: no memory for new symbol table entry\n");
        exit(EXIT_FAILURE);
    }

    memset(ent, 0, sizeof(struct symtabEntryS));
    ent->name = malloc(strlen(symb) + 1);

    if (!ent->name)
    {
        fprintf(stderr, "fatal error: no memory for new symbol table entry\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ent->name, symb);

    if (!ns->first)
    {
        ns->first = ent;
        ns->last = ent;
    }
    else
    {
        ns->last->next = ent;
        ns->last = ent;
    }

    return ent;
}

int remFromSymtab(struct symtabEntryS *ent, struct symtabS *st)
// Used to remove entries from linked list
{
    int result = 0;
    if (ent->val)
    {
        free(ent->val);
    }

    if (ent->funcBody)
    {
        freeNodeTree(ent->funcBody);
    }

    free(ent->name);

    if (st->first == ent)
    {
        st->first = st->first->next;

        if (st->last == ent)
        {
            st->last = NULL;
        }
        result = 1;
    }
    else
    {
        struct symtabEntryS *temp1 = st->first;
        struct symtabEntryS *temp2 = NULL;

        while (temp1 && temp1 != ent)
        {
            temp2 = temp1;
            temp1 = temp1->next;
        }

        if (temp1 == ent)
        {
            temp2->next = ent->next;
            result = 1;
        }
    }

    free(ent);
    return result;
}

struct symtabEntryS *doLookup(char *str, struct symtabS *sTable)
// Searches passed parameter linked list for a passed parameter entry
{
    if (!str || !sTable)
    {
        return NULL;
    }

    struct symtabEntryS *ent = sTable->first;

    while (ent)
    {
        if (strcmp(ent->name, str) == 0)
        {
            return ent;
        }
        ent = ent->next;
    }

    return NULL;
}

struct symtabEntryS *getSymtabEntry(char *str)
// Searches table for entry match based on name
{
    int temp = symtabStack.symtabCount - 1;

    do
    {
        struct symtabS *st = symtabStack.symtabList[temp];
        struct symtabEntryS *ent = doLookup(str, st);

        if (ent)
        {
            return ent;
        }

    } while (--temp >= 0);

    return NULL;
}