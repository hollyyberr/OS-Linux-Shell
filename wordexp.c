#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <pwd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include "shell.h"
#include "symtab/symtab.h"
#include "executor.h"

#define INVALID_VAR ((char *)-1)

// Conversion for word to command token
struct wordS *makeWord(char *s)
{
    struct wordS *w = malloc(sizeof(struct wordS));
    if (!w)
    {
        return NULL;
    }
    size_t length = strlen(s);
    char *d = malloc(length + 1);
    if (!d)
    {
        free(w);
        return NULL;
    }
    strcpy(d, s);
    w->data = d;
    w->length = length;
    w->next = NULL;
    return w;
}

// Free previously used memory
void freeAllWords(struct wordS *v1)
{
    while (v1)
    {
        struct wordS *d = v1;
        v1 = v1->next;

        if (d->data)
        {
            free(d->data);
        }
        free(d);
    }
}

// Convert token tree to command string
char *wordlistToStr(struct wordS *w)
{
    if (!w)
    {
        return NULL;
    }
    size_t length = 0;
    struct wordS *w2 = w;
    while (w2)
    {
        length += w2->length + 1;
        w2 = w2->next;
    }
    char *s = malloc(length + 1);
    if (!s)
    {
        return NULL;
    }
    char *s2 = s;
    w2 = w;
    while (w2)
    {
        sprintf(s2, "%s ", w2->data);
        s2 += w2->length + 1;
        w2 = w2->next;
    }
    s2[-1] = '\0';
    return s;
}

// Delete character at index param
void deleteCharAt(char *s, size_t ind)
{
    char *a = s + ind;
    char *b = a + 1;
    while ((*a++ = *b++))
    {
        ;
    }
}

// Validates string parameter
int isName(char *s)
{
    if (!isalpha(*s) && *s != '_')
    {
        return 0;
    }
    while (*++s)
    {
        if (!isalnum(*s) && *s != '_')
        {
            return 0;
        }
    }
    return 1;
}

// Matches closing + opening quotes
size_t findClosingQuote(char *d)
{
    char q = d[0];
    if (q != '\'' && q != '"' && q != '`')
    {
        return 0;
    }
    size_t i = 0, length = strlen(d);
    while (++i < length)
    {
        if (d[i] == q)
        {
            if (d[i - 1] == '\\')
            {
                if (q != '\'')
                {
                    continue;
                }
            }
            return i;
        }
    }
    return 0;
}

// Matches closing + opening braces
size_t findClosingBrace(char *d)
{
    char openBrace = d[0], closeBrace;
    if (openBrace != '{' && openBrace != '(')
    {
        return 0;
    }
    if (openBrace == '{')
    {
        closeBrace = '}';
    }
    else
    {
        closeBrace = ')';
    }
    size_t openCount = 1, closeCount = 0;
    size_t i = 0, length = strlen(d);
    while (++i < length)
    {
        if ((d[i] == '"') || (d[i] == '\'') || (d[i] == '`'))
        {
            if (d[i - 1] == '\\')
            {
                continue;
            }
            char q = d[i];
            while (++i < length)
            {
                if (d[i] == q && d[i - 1] != '\\')
                {
                    break;
                }
            }
            if (i == length)
            {
                return 0;
            }
            continue;
        }
        if (d[i - 1] != '\\')
        {
            if (d[i] == openBrace)
            {
                openCount++;
            }
            else if (d[i] == closeBrace)
            {
                closeCount++;
            }
        }
        if (openCount == closeCount)
        {
            break;
        }
    }
    if (openCount != closeCount)
    {
        return 0;
    }
    return i;
}

// Substitute strings from start to end
char *substituteStr(char *a, char *b, size_t aSize, size_t bSize)
{
    char bef[aSize + 1];
    strncpy(bef, a, aSize);
    bef[aSize] = '\0';
    size_t afterLength = strlen(a) - bSize + 1;
    char aft[afterLength];
    strcpy(aft, a + bSize + 1);
    size_t totalLength = aSize + bSize + strlen(b);
    char *fin = malloc(totalLength + 1);
    if (!fin)
    {
        fprintf(stderr, "Error: Not enough memory to perform variable substitution\n");
        return NULL;
    }
    if (!totalLength)
    {
        fin[0] = '\0';
    }
    else
    {
        strcpy(fin, bef);
        strcat(fin, b);
        strcat(fin, aft);
    }
    return fin;
}

int substituteWord(char **begin, char **a, size_t length, char *(func)(char *), int addQuotes)
{
    char *t = malloc(length + 1);
    if (!t)
    {
        (*a) += length;
        return 0;
    }
    strncpy(t, *a, length);
    t[length--] = '\0';
    char *t2;
    if (func)
    {
        t2 = func(t);
        if (t2 == INVALID_VAR)
        {
            t2 = NULL;
        }
        if (t2)
        {
            free(t);
        }
    }
    else
    {
        t2 = t;
    }

    if (!t2)
    {
        (*a) += length;
        free(t);
        return 0;
    }

    size_t i = (*a) - (*begin);

    t = quoteVal(t2, addQuotes);
    free(t2);
    if (t)
    {
        if ((t2 = substituteStr(*begin, t, i, i + length)))
        {
            free(*begin);
            (*begin) = t2;
            length = strlen(t);
        }
        free(t);
    }

    (*a) = (*begin) + i + length - 1;
    return 1;
}

// Expand single word
struct wordS *wordExpand(char *oWord)
{
    if (!oWord)
    {
        return NULL;
    }
    if (!*oWord)
    {
        return makeWord(oWord);
    }
    char *begin = malloc(strlen(oWord) + 1);
    if (!begin)
    {
        return NULL;
    }
    strcpy(begin, oWord);

    char *a = begin, *b;
    char *t;
    char let;
    size_t i = 0;
    size_t length;
    int doubleQuotes = 0;
    int varAssign = 0;
    int varAssignEQ = 0;
    int expd = 0;
    char *(*func)(char *);

    do
    {
        switch (*a)
        {
        case '~':
            if (doubleQuotes)
            {
                break;
            }
            if (a == begin || (varAssign && (a[-1] == ':' || (a[-1] == '=' && varAssignEQ == 1))))
            {
                int tildeQuoted = 0;
                int end = 0;
                b = a + 1;

                while (*b)
                {
                    switch (*b)
                    {
                    case '\\':
                        tildeQuoted = 1;
                        b++;
                        break;

                    case '"':
                    case '\'':
                        i = findClosingQuote(b);
                        if (i)
                        {
                            tildeQuoted = 1;
                            b += i;
                        }
                        break;

                    case '/':
                        end = 1;
                        break;

                    case ':':
                        if (varAssign)
                        {
                            end = 1;
                        }
                        break;
                    }
                    if (end)
                    {
                        break;
                    }
                    b++;
                }
                if (tildeQuoted)
                {
                    a = b;
                    break;
                }
                length = b - a;
                substituteWord(&begin, &a, length, tildeExpand, !doubleQuotes);
                expd = 1;
            }
            break;

        case '"':
            doubleQuotes = !doubleQuotes;
            break;
        case '=':
            if (doubleQuotes)
            {
                break;
            }
            length = a - begin;
            t = malloc(length + 1);
            if (!t)
            {
                fprintf(stderr, "Error: Not enough memory for internal buffers\n");
                break;
            }
            strncpy(t, begin, length);
            t[length] = '\0';

            if (isName(t))
            {
                varAssign = 1;
                varAssignEQ++;
            }
            free(t);
            break;

        case '\\':
            a++;
            break;
        case '\'':
            if (doubleQuotes)
            {
                break;
            }

            a += findClosingQuote(a);
            break;

        case '`':
            if ((length = findClosingQuote(a)) == 0)
            {
                break;
            }
            substituteWord(&begin, &a, length + 1, commandSubstitute, 0);
            expd = 1;
            break;

        case '$':
            let = a[1];
            switch (let)
            {
            case '{':
                if ((length = findClosingBrace(a + 1)) == 0)
                {
                    break;
                }

                if (!substituteWord(&begin, &a, length + 2, varExpand, 0))
                {
                    free(begin);
                    return NULL;
                }
                expd = 1;
                break;

            case '(':
                i = 0;
                if (a[2] == '(')
                {
                    i++;
                }
                if ((length = findClosingBrace(a + 1)) == 0)
                {
                    break;
                }
                func = i ? arithmExpand : commandSubstitute;
                substituteWord(&begin, &a, length + 2, func, 0);
                expd = 1;
                break;

            default:
                if (!isalpha(a[1]) && a[1] != '_')
                {
                    break;
                }
                b = a + 1;
                while (*b)
                {
                    if (!isalnum(*b) && *b != '_')
                    {
                        break;
                    }
                    b++;
                }
                if (b == a + 1)
                {
                    break;
                }
                substituteWord(&begin, &a, b - a, varExpand, 0);
                expd = 1;
                break;
            }
            break;
        default:
            if (isspace(*a) && !doubleQuotes)
            {
                expd = 1;
            }
            break;
        }
    } while (*(++a));

    struct wordS *ws = NULL;
    if (expd)
    {
        ws = fieldSplit(begin);
    }

    if (!ws)
    {
        ws = makeWord(begin);
        if (!ws)
        {
            fprintf(stderr, "Error: Not enough memory\n");
            free(begin);
            return NULL;
        }
    }
    free(begin);
    ws = pathnamesExpand(ws);
    removeQuotes(ws);
    return ws;
}

// Tilde expansion on character
char *tildeExpand(char *a)
{
    char *h = NULL;
    size_t length = strlen(a);
    char *b = NULL;
    struct symtabEntryS *ent;

    if (length == 1)
    {
        ent = getSymtabEntry("HOME");
        if (ent && ent->val)
        {
            h = ent->val;
        }
        else
        {
            struct passwd *p;
            p = getpwuid(getuid());
            if (p)
            {
                h = p->pw_dir;
            }
        }
    }
    else
    {
        struct passwd *p;
        p = getpwnam(a + 1);
        if (p)
        {
            h = p->pw_dir;
        }
    }

    if (!h)
    {
        return NULL;
    }

    b = malloc(strlen(h) + 1);
    if (!b)
    {
        return NULL;
    }
    strcpy(b, h);
    return b;
}

// Variable Expansion
char *varExpand(char *oVarName)
{
    if (!oVarName)
    {
        return NULL;
    }
    oVarName++;
    size_t length = strlen(oVarName);
    if (*oVarName == '{')
    {
        oVarName[length - 1] = '\0';
        oVarName++;
    }

    if (!*oVarName)
    {
        return NULL;
    }

    int gLen = 0;
    if (*oVarName == '#')
    {
        if (strchr(oVarName, ':'))
        {
            fprintf(stderr, "Error: Cannot substitute variable: %s\n", oVarName);
            return INVALID_VAR;
        }
        gLen = 1;
        oVarName++;
    }

    if (!*oVarName)
    {
        return NULL;
    }

    char *subst = strchr(oVarName, ':');
    if (!subst)
    {
        subst = strchrAny(oVarName, "-=?+%#");
    }

    length = subst ? (size_t)(subst - oVarName) : strlen(oVarName);

    if (subst && *subst == ':')
    {
        subst++;
    }

    char varChar[length + 1];
    strncpy(varChar, oVarName, length);
    varChar[length] = '\0';

    char *empVal = "";
    char *temp = NULL;
    char set = 0;

    struct symtabEntryS *ent = getSymtabEntry(varChar);
    temp = (ent && ent->val && ent->val[0]) ? ent->val : empVal;

    if (!temp || temp == empVal)
    {
        if (subst && *subst)
        {
            switch (subst[0])
            {
            case '-':
                temp = subst + 1;
                break;

            case '=':
                temp = subst + 1;
                set = 1;
                break;

            case '?':
                if (subst[1] == '\0')
                {
                    fprintf(stderr, "error: %s: Parameter is not set\n", varChar);
                }
                else
                {
                    fprintf(stderr, "error: %s: %s\n", varChar, subst + 1);
                }
                return INVALID_VAR;

            case '+':
                return NULL;

            case '#':
            case '%':
                break;

            default:
                return INVALID_VAR;
            }
        }
        else
        {
            temp = empVal;
        }
    }
    else
    {
        if (subst && *subst)
        {
            switch (subst[0])
            {
            case '-':
            case '=':
            case '?':
                break;

            case '+':
                temp = subst + 1;
                break;

            case '%':
                subst++;
                char *c = wordExpandToStr(temp);
                if (!c)
                {
                    return INVALID_VAR;
                }

                int lng = 0;
                if (*subst == '%')
                {
                    lng = 1;
                    subst++;
                }

                if ((length = matchSuffix(subst, c, lng)) == 0)
                {
                    return c;
                }

                char *c2 = malloc(length + 1);
                if (c2)
                {
                    strncpy(c2, c, length);
                    c2[length] = '\0';
                }
                free(c);
                return c2;

            case '#':
                subst++;
                c = wordExpandToStr(temp);

                if (!c)
                {
                    return INVALID_VAR;
                }

                lng = 0;
                if (*subst == '#')
                {
                    lng = 1;
                    subst++;
                }

                if ((length = matchPrefix(subst, c, lng)) == 0)
                {
                    return c;
                }

                c2 = malloc(strlen(c) - length + 1);
                if (c2)
                {
                    strcpy(c2, c + length);
                }
                free(c);
                return c2;

            default:
                return INVALID_VAR;
            }
        }
    }

    int expd = 0;
    if (temp)
    {
        if ((temp = wordExpandToStr(temp)))
        {
            expd = 1;
        }
    }

    if (set)
    {
        if (!ent)
        {
            ent = addToSymtab(varChar);
        }

        if (ent)
        {
            symtabEntrySetval(ent, temp);
        }
    }

    char buffer[32];
    char *c = NULL;
    if (gLen)
    {
        if (!temp)
        {
            sprintf(buffer, "0");
        }
        else
        {
            sprintf(buffer, "%lu", strlen(temp));
        }
        c = malloc(strlen(buffer) + 1);
        if (c)
        {
            strcpy(c, buffer);
        }
    }
    else
    {
        c = malloc(strlen(temp) + 1);
        if (c)
        {
            strcpy(c, temp);
        }
    }

    if (expd)
    {
        free(temp);
    }

    return c ?: INVALID_VAR;
}

// Command Substitution
char *commandSubstitute(char *oCommand)
{
    char buffer[1024];
    size_t bufferSize = 0;
    char *bufferChar = NULL;
    char *a = NULL;
    int i = 0;
    int bQuote = (*oCommand == '`');
    char *command = malloc(strlen(oCommand + 1));

    if (!command)
    {
        fprintf(stderr, "Error: Not enough memory to perform command substitution\n");
        return NULL;
    }

    strcpy(command, oCommand + (bQuote ? 1 : 2));
    char *command2 = command;
    size_t commandLen = strlen(command);

    if (bQuote)
    {
        if (command[commandLen - 1] == '`')
        {
            command[commandLen - 1] = '\0';
        }
        char *a1 = command;
        do
        {
            if (*a1 == '\\' && (a1[1] == '$' || a1[1] == '`' || a1[1] == '\\'))
            {
                char *b2 = a1;
                char *c3 = a1 + 1;
                while ((*b2++ = *c3++))
                {
                    ;
                }
            }
        } while (*(++a1));
    }
    else
    {
        if (command[commandLen - 1] == ')')
        {
            command[commandLen - 1] = '\0';
        }
    }

    FILE *fileP = popen(command2, "r");

    if (!fileP)
    {
        free(command2);
        fprintf(stderr, "Error: Could not open pipe: %s\n", strerror(errno));
        return NULL;
    }

    while ((i = fread(buffer, 1, 1024, fileP)))
    {
        if (!bufferChar)
        {
            bufferChar = malloc(i + 1);
            if (!bufferChar)
            {
                goto fin;
            }
            a = bufferChar;
        }
        else
        {
            char *bufferChar2 = realloc(bufferChar, bufferSize + i + 1);

            if (!bufferChar2)
            {
                free(bufferChar);
                bufferChar = NULL;
                goto fin;
            }

            bufferChar = bufferChar2;
            a = bufferChar + bufferSize;
        }
        bufferSize += i;
        memcpy(a, buffer, i);
        a[i] = '\0';
    }
    if (!bufferSize)
    {
        free(command2);
        return NULL;
    }
    i = bufferSize - 1;

    while (bufferChar[i] == '\n' || bufferChar[i] == '\r')
    {
        bufferChar[i] = '\0';
        i--;
    }

fin:
    // Close pipe
    pclose(fileP);

    // Free mem
    free(command2);

    if (!bufferChar)
    {
        fprintf(stderr, "Error: Not enough memory to perform command substitution\n");
    }

    return bufferChar;
}

// Check if char is a valid character
static inline int isIFSchar(char a, char *IFS)
{
    if (!*IFS)
    {
        return 0;
    }
    do
    {
        if (a == *IFS)
        {
            return 1;
        }
    } while (*++IFS);
    return 0;
}

// Skip whitespace
void skipIFSwhitespace(char **s1, char *IFS)
{
    char *IFS2 = IFS;
    char *s2 = *s1;

    do
    {
        if (*s2 == *IFS2)
        {
            s2++;
            IFS2 = IFS - 1;
        }
    } while (*++IFS2);

    *s1 = s2;
}

// Skip delimiters
void skipIFSdelimiters(char *s, char *IFSspace, char *IFSdelim, size_t *ind, size_t length)
{
    size_t i = *ind;
    while ((i < length) && isIFSchar(s[i], IFSspace))
    {
        i++;
    }
    while ((i < length) && isIFSchar(s[i], IFSdelim))
    {
        i++;
    }
    while ((i < length) && isIFSchar(s[i], IFSspace))
    {
        i++;
    }
    *ind = i;
}

// Coverts words to separate fields
struct wordS *fieldSplit(char *s)
{
    struct symtabEntryS *ent = getSymtabEntry("IFS");
    char *IFS = ent ? ent->val : NULL;
    char *a;

    if (!IFS)
    {
        IFS = " \t\n";
    }
    if (IFS[0] == '\0')
    {
        return NULL;
    }

    char IFSspace[64];
    char IFSdelim[64];

    if (strcmp(IFS, " \t\n") == 0)
    {
        IFSspace[0] = ' ';
        IFSspace[1] = '\t';
        IFSspace[2] = '\n';
        IFSspace[3] = '\0';
        IFSdelim[0] = '\0';
    }
    else
    {
        a = IFS;
        char *spaceA = IFSspace;
        char *delimA = IFSdelim;
        do
        {
            if (isspace(*a))
            {
                *spaceA++ = *a++;
            }
            else
            {
                *delimA++ = *a++;
            }
        } while (*a);
        *spaceA = '\0';
        *delimA = '\0';
    }

    size_t length = strlen(s);
    size_t x = 0;
    size_t y = 0;
    size_t z;
    int flds = 1;
    char q = 0;

    skipIFSwhitespace(&s, IFSspace);

    do
    {
        switch (s[x])
        {
        case '\\':
            if (q != '\'')
            {
                x++;
            }
            break;

        case '\'':
        case '"':
        case '`':
            if (q == s[x])
            {
                q = 0;
            }
            else
            {
                q = s[x];
            }
            break;

        default:
            if (q)
            {
                break;
            }
            if (isIFSchar(s[x], IFSspace) || isIFSchar(s[x], IFSdelim))
            {
                skipIFSdelimiters(s, IFSspace, IFSdelim, &x, length);
                if (x < length)
                {
                    flds++;
                }
            }
            break;
        }
    } while (++x < length);

    if (flds == 1)
    {
        return NULL;
    }
    struct wordS *fld1 = NULL;
    struct wordS *cursor = NULL;

    x = 0;
    y = 0;
    q = 0;

    do
    {
        switch (s[x])
        {
        case '\\':
            if (q != '\'')
            {
                x++;
            }
            break;

        case '\'':
            a = s + x + 1;
            while (*a && *a != '\'')
            {
                a++;
            }
            x = a - s;
            break;

        case '"':
        case '`':
            if (q == s[x])
            {
                q = 0;
            }
            else
            {
                q = s[x];
            }
            break;

        default:
            if (q)
            {
                break;
            }
            if (isIFSchar(s[x], IFSspace) || isIFSchar(s[x], IFSdelim) || (x == length))
            {
                char *temp = malloc(x - y + 1);

                if (!temp)
                {
                    fprintf(stderr, "Error: Not enough memory for field splitting\n");
                    return fld1;
                }

                strncpy(temp, s + y, x - y);
                temp[x - y] = '\0';
                struct wordS *field = malloc(sizeof(struct wordS));

                if (!field)
                {
                    free(temp);
                    return fld1;
                }

                field->data = temp;
                field->length = x - y;
                field->next = NULL;

                if (!fld1)
                {
                    fld1 = field;
                }
                if (!cursor)
                {
                    cursor = field;
                }
                else
                {
                    cursor->next = field;
                    cursor = field;
                }

                z = x;
                skipIFSdelimiters(s, IFSspace, IFSdelim, &x, length);
                y = x;

                if (x != z && x < length)
                {
                    x--;
                }
            }
            break;
        }
    } while (++x <= length);

    return fld1;
}

// Pathname expansion
struct wordS *pathnamesExpand(struct wordS *ws)
{
    struct wordS *w1 = ws;
    struct wordS *w2 = NULL;

    while (w1)
    {
        char *a = w1->data;
        if (!hasGlobChars(a, strlen(a)))
        {
            w2 = w1;
            w1 = w1->next;
            continue;
        }

        glob_t g;
        char **matches = getFileNameMatches(a, &g);

        if (!matches || !matches[0])
        {
            globfree(&g);
        }
        else
        {
            struct wordS *top = NULL;
            struct wordS *bottom = NULL;

            for (size_t y = 0; y < g.gl_pathc; y++)
            {
                if (matches[y][0] == '.' && (matches[y][1] == '.' || matches[y][1] == '\0' || matches[y][1] == '/'))
                {
                    continue;
                }
                if (!top)
                {
                    top = makeWord(matches[y]);
                    bottom = top;
                }
                else
                {
                    bottom->next = makeWord(matches[y]);
                    if (bottom->next)
                    {
                        bottom = bottom->next;
                    }
                }
            }

            if (w1 == ws)
            {
                ws = top;
            }
            else if (w2)
            {
                w2->next = top;
            }

            w2 = bottom;
            bottom->next = w1->next;
            w1->next = NULL;
            freeAllWords(w1);
            w1 = bottom;
            globfree(&g);
        }

        w2 = w1;
        w1 = w1->next;
    }

    return ws;
}

// Quote removal
void removeQuotes(struct wordS *wList)
{
    if (!wList)
    {
        return;
    }

    int doubleQuotes = 0;
    struct wordS *w1 = wList;
    char *a;

    while (w1)
    {
        a = w1->data;
        while (*a)
        {
            switch (*a)
            {
            case '"':
                doubleQuotes = !doubleQuotes;
                deleteCharAt(a, 0);
                break;

            case '\'':
                if (doubleQuotes)
                {
                    a++;
                    break;
                }

                deleteCharAt(a, 0);
                while (*a && *a != '\'')
                {
                    a++;
                }
                if (*a == '\'')
                {
                    deleteCharAt(a, 0);
                }
                break;

            case '`':
                deleteCharAt(a, 0);
                break;

            case '\v':
            case '\f':
            case '\t':
            case '\r':
            case '\n':
                a++;
                break;

            case '\\':
                if (doubleQuotes)
                {
                    switch (a[1])
                    {
                    case '$':
                    case '`':
                    case '"':
                    case '\\':
                    case '\n':
                        deleteCharAt(a, 0);
                        a++;
                        break;

                    default:
                        a++;
                        break;
                    }
                }
                else
                {
                    deleteCharAt(a, 0);
                    a++;
                }
                break;

            default:
                a++;
                break;
            }
        }

        w1->length = strlen(w1->data);
        w1 = w1->next;
    }
}

char *wordExpandToStr(char *w)
{
    struct wordS *w1 = wordExpand(w);
    if (!w1)
    {
        return NULL;
    }

    char *result = wordlistToStr(w1);
    freeAllWords(w1);

    return result;
}