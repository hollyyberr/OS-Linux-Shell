#ifndef SHELL_H
#define SHELL_H

// Source code for shell activities
void printPrompt1(void);
void printPrompt2(void);

char *readCommand(void);
void initsh(void);

#include "source.h"
int parseAndExecute(struct sourceS *src);

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
}

struct wordS *
makeWord(char *str);
void freeAllWords(struct wordS *first);

char *wordlistToStr(struct wordS *word);
void deleteCharAt(char *str, sizeT ind);
int isName(char *str);
sizeT findClosingQuote(char *data);
sizeT findClosingBrace(char *data);
char *substituteStr(char *s1, char *s2, sizeT start, sizeT end);
int substituteWord(char **pstart, char **p, sizeT length, char *(func)(char *), int addQuotes);
char *strchrAny(char *string, char *chars);
char *quoteVal(char *val, int addQuotes);
int checkBufferBounds(int *count, int *length, char ***buffer);
void freeBuffer(int length, char **buffer);
char *tildeExpand(char *s);
char *varExpand(char *origvarName);
char *commandSubstitute(char *origCommand);
char *arithmEEpand(char *expr);
struct wordS *fieldSplit(char *str);
struct wordS *pathnamesExpand(struct wordS *words);
void removeQuotes(struct wordS *wordlist);
struct wordS *wordExpand(char *origWord);

#endif