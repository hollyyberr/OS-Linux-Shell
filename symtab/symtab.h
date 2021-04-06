#ifndef SYMTAB_H
#define SYMTAB_H

#include "../node.h"

#define MAX_SYMTAB 256

enum symbolTypeE
{
    SYM_STR,
    SYM_FUNC,
};

struct symtabEntryS
/* Skeleton for base data structure */
{
    char *name;
    enum symbolTypeE valType;
    char *val;
    unsigned int flags;
    struct symtabEntryS *next;
    struct nodeS *funcBody;
};

struct symtabS
/* Structure for individual table design */
{
    int lvl;
    struct symtabEntryS *first, *last;
};

#define FLAG_EXPORT (1 << 0)

struct symtabStackS
/* Structure for symbol table stack */
{
    int symtabCount;
    struct symtabS *symtabList[MAX_SYMTAB];
    struct symtabS *globalSymtab, *localSymtab;
};

struct symtabS *newSymtab(int level);
struct symtabS *symtabStackPush(void);
struct symtabS *symtabStackPop(void);
int remFromSymtab(struct symtabEntryS *entry, struct symtabS *symtab);
struct symtabEntryS *addToSymtab(char *symbol);
struct symtabEntryS *doLookup(char *str, struct symtabS *symtable);
struct symtabEntryS *getSymtabEntry(char *str);
struct symtabS *getLocalSymtab(void);
struct symtabS *getGlobalSymtab(void);
struct symtabStackS *getSymtabStack(void);
void initSymtab(void);
void dumpLocalSymtab(void);
void freeSymtab(struct symtabS *symtab);
void symtabEntrySetval(struct symtabEntryS *entry, char *val);

#endif