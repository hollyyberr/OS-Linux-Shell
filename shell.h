#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>
#include <glob.h>
#include "source.h"

// Source code for shell activities
void printPrompt1(void);
void printPrompt2(void);

char *readCommand(void);
int parseAndExecute(struct sourceS *src);

void initsh(void);

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

struct wordS
{
    char *data;
    int length;
    struct wordS *next;
};

// Wordexp.c
struct wordS *makeWord(char *s);

void freeAllWords(struct wordS *v1);

size_t findClosingQuote(char *d);

size_t findClosingBrace(char *d);

void deleteCharAt(char *s, size_t ind);

char *substituteStr(char *a, char *b, size_t aSize, size_t bSize);

char *wordlistToStr(struct wordS *w);

struct wordS *wordExpand(char *oWord);

char *wordExpandToStr(char *w);

char *tildeExpand(char *a);

char *commandSubstitute(char *oCommand);

char *varExpand(char *oVarName);

char *posParamsExpand(char *temp, int doubleQuotes);

struct wordS *pathnamesExpand(struct wordS *ws);

struct wordS *fieldSplit(char *s);

void removeQuotes(struct wordS *wList);

// Shunt.c
char *arithmExpand(char *origExp);

// Strings.c
char *strchrAny(char *s, char *cs);

char *quoteVal(char *v, int addQuotes);

int checkBufferBounds(int *cnt, int *length, char ***buffer);

void freeBuffer(int length, char **buffer);

// Pattern.c
int hasGlobChars(char *a, size_t length);

int matchPrefix(char *ptrn, char *seg, int lng);

int matchSuffix(char *ptrn, char *a, int lng);

char **getFileNameMatches(char *ptrn, glob_t *matches);

#endif