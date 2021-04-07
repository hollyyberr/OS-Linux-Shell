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

struct wordS *makeWord(char *s);
void freeAllWords(struct wordS *v1);

char *wordlistToStr(struct wordS *w);
void deleteCharAt(char *s, size_t ind);
int isName(char *s);
size_t findClosingQuote(char *d);
size_t findClosingBrace(char *d);
char *substituteStr(char *a, char *b, size_t aSize, size_t bSize);
int substituteWord(char **begin, char **a, size_t length, char *(func)(char *), int addQuotes);
char *strchrAny(char *s, char *cs);
char *quoteVal(char *v, int addQuotes);
int checkBufferBounds(int *cnt, int *length, char ***buffer);
void freeBuffer(int length, char **buffer);
char *tildeExpand(char *a);
char *varExpand(char *oVarName);
char *commandSubstitute(char *oCommand);
char *arithmExpand(char *exp);
struct wordS *fieldSplit(char *s);
struct wordS *pathnamesExpand(struct wordS *ws);
void removeQuotes(struct wordS *wList);
struct wordS *wordExpand(char *oWord);

#endif