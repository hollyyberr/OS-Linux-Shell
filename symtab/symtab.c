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
{
    symtabStack.symtabCount = 1;
    symtabLevel = 0;

    struct symtabS *globalSymtab = malloc(sizeof(struct symtabS));
    
    if(!globalSymtab)
    {
        fprintf(stderr, "Fatal Error: Not enought memory for global symbol table\n");
        exit(EXIT_FAILURE);
    }
    
    memset(globalSymtab, 0, sizeof(struct symtabS));
    symtabStack.globalSymtab = globalSymtab;
    symtabStack.localSymtab = globalSymtab;
    symtabStack.symtabList[0] = globalSymtab;
    globalSymtab->lvl = 0;
}


struct symtabS *newSymtab(int lvl)
{
    struct symtabS *symtab = malloc(sizeof(struct symtabS));
    
    if(!symtab)
    {
        fprintf(stderr, "Fatal Error: Not enough memory for new symbol table\n");
        exit(EXIT_FAILURE);
    }
    
    memset(symtab, 0, sizeof(struct symtabS));
    symtab->lvl = lvl;
    return symtab;
}


void freeSymtab(struct symtabS *symtab)
{
    if(symtab == NULL)
    {
        return;
    }
    
    struct symtabEntryS *ent = symtab->first;
    
    while(ent)
    {
        if(ent->name)
        {
            free(ent->name);
        }
    
        if(ent->val)
        {
            free(ent->val);
        }
    
        if(ent->funcBody)
        {
            freeNodeTree(ent->funcBody);
        }
    
    	struct symtabEntryS *next = ent->next;
        free(ent);
        ent = next;
    }
    
    free(symtab);
}


void dumpLocalSymtab(void)
{
    struct symtabS *symtab = symtabStack.localSymtab;
    int i = 0;
    int ind = symtab->lvl * 4;
    
    fprintf(stderr, "%*sSymbol table [Level %d]:\r\n", ind, " ", symtab->lvl);
    fprintf(stderr, "%*s===========================\r\n", ind, " ");
    fprintf(stderr, "%*s  No               Symbol                    Val\r\n", ind, " ");
    fprintf(stderr, "%*s------ -------------------------------- ------------\r\n", ind, " ");
    
    struct symtabEntryS *ent = symtab->first;
    
    while(ent)
    {
        fprintf(stderr, "%*s[%04d] %-32s '%s'\r\n", ind, " ",
                i++, ent->name, ent->val);
        ent = ent->next;
    }
    
    fprintf(stderr, "%*s------ -------------------------------- ------------\r\n", ind, " ");
}


struct symtabEntryS *addToSymtab(char *symb)
{
    if(!symb || symb[0] == '\0')
    {
        return NULL;
    }

    struct symtabS *temp = symtabStack.localSymtab;
    struct symtabEntryS *ent = NULL;
    
    if((ent = doLookup(symb, temp)))
    {
        return ent;
    }
    
    ent = malloc(sizeof(struct symtabEntryS));
    
    if(!ent)
    {
        fprintf(stderr, "Fatal Error: Not enough memory for new symbol table entry\n");
        exit(EXIT_FAILURE);
    }
    
    memset(ent, 0, sizeof(struct symtabEntryS));
    ent->name = malloc(strlen(symb)+1);
    
    if(!ent->name)
    {
        fprintf(stderr, "Fatal Error: Not enough memory for new symbol table entry\n");
        exit(EXIT_FAILURE);
    }
    
    strcpy(ent->name, symb);
    
    if(!temp->first)
    {
        temp->first = ent;
        temp->last = ent;
    }
    else
    {
        temp->last->next = ent;
        temp->last = ent;
    }
    
    return ent;
}


struct symtabEntryS *doLookup(char *str, struct symtabS *table)
{
    if(!str || !table)
    {
        return NULL;
    }

    struct symtabEntryS *ent = table->first;
    
    while(ent)
    {
        if(strcmp(ent->name, str) == 0)
        {
            return ent;
        }
        ent = ent->next;
    }
    
    return NULL;
}


struct symtabEntryS *getSymtabEntry(char *str)
{
    int i = symtabStack.symtabCount-1;
    
    do
    {
        struct symtabS *symtab = symtabStack.symtabList[i];
        struct symtabEntryS *ent = doLookup(str, symtab);
    
    	if(ent)
        {
            return ent;
        }
    
    } while(--i >= 0);
    
    return NULL;
}


void symtabEntrySetval(struct symtabEntryS *ent, char *val)
{
    if(ent->val)
    {
        free(ent->val);
    }

    if(!val)
    {
        ent->val = NULL;
    }
    else
    {
        char *temp = malloc(strlen(val)+1);
    
    	if(temp)
        {
            strcpy(temp, val);
        }
        else
        {
            fprintf(stderr, "error: Not enough memory for symbol table entry's value\n");
        }
    
    	ent->val = temp;
    }
}


int remFromSymtab(struct symtabEntryS *ent, struct symtabS *symtab)
{
    int res = 0;
    if(ent->val)
    {
        free(ent->val);
    }

    if(ent->funcBody)
    {
        freeNodeTree(ent->funcBody);
    }
    
    free(ent->name);
    
    if(symtab->first == ent)
    {
        symtab->first = symtab->first->next;
    
    	if(symtab->last == ent)
        {
            symtab->last = NULL;
        }
        res = 1;
    }
    else
    {
        struct symtabEntryS *a = symtab->first;
        struct symtabEntryS *b = NULL;
    
    	while(a && a != ent)
        {
            b = a;
            a = a->next;
        }
    
    	if(a == ent)
        {
            b->next = ent->next;
            res = 1;
        }
    }
    
    free(ent);
    return res;
}


void symtabStackAdd(struct symtabS *symtab)
{
    symtabStack.symtabList[symtabStack.symtabCount++] = symtab;
    symtabStack.localSymtab = symtab;
}


struct symtabS *symtabStackPush(void)
{
    struct symtabS *st = newSymtab(++symtabLevel);
    symtabStackAdd(st);
    return st;
}


struct symtabS *symtabStackPop(void)
{
    if(symtabStack.symtabCount == 0)
    {
        return NULL;
    }

    struct symtabS *st = symtabStack.symtabList[symtabStack.symtabCount-1];
    
    symtabStack.symtabList[--symtabStack.symtabCount] = NULL;
    symtabLevel--;
    
    if(symtabStack.symtabCount == 0)
    {
        symtabStack.localSymtab  = NULL;
        symtabStack.globalSymtab = NULL;
    }
    else
    {
        symtabStack.localSymtab = symtabStack.symtabList[symtabStack.symtabCount-1];
    }
    
    return st;
}


struct symtabS *getLocalSymtab(void)
{
    return symtabStack.localSymtab;
}


struct symtabS *getGlobalSymtab(void)
{
    return symtabStack.globalSymtab;
}


struct symtabStackS *getSymtabStack(void)
{
    return &symtabStack;
}