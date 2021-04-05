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

struct wordS *makeWord(char *str)
{

    struct wordS *word = malloc(sizeof(struct wordS));
    if (!word)
    {
        return NULL;
    }

    /* alloc string memory */
    size_t length = strlen(str);
    char *data = malloc(length + 1);

    if (!data)
    {
        free(word);
        return NULL;
    }

    /* copy string */
    strcpy(data, str);
    word->data = data;
    word->length = length;
    word->next = NULL;

    /* return struct */
    return word;
}

/*
 * free the memory used by a list of words.
 */
void freeAllWords(struct wordS *f)
{
    while (f)
    {
        struct wordS *del = f;
        f = f->next;

        if (del->data)
        {
            /* free the word text */
            free(del->data);
        }

        /* free the word */
        free(del);
    }
}

char *wordlistToTtr(struct wordS *word)
{
    if (!word)
    {
        return NULL;
    }
    size_t length = 0;
    struct wordS *temp = word;
    while (temp)
    {
        length += temp->length + 1;
        temp = temp->next;
    }
    char *str = malloc(length + 1);
    if (!str)
    {
        return NULL;
    }
    char *temp2 = str;
    temp = word;
    while (temp)
    {
        sprintf(temp2, "%s ", temp->data);
        temp2 += temp->length + 1;
        temp = temp->next;
    }
    temp2[-1] = '\0';
    return str;
}

void deleteCharAt(char *str, size_t ind)
{
    char *temp1 = str + ind;
    char *temp2 = temp1 + 1;
    while ((*temp1++ = *temp2++))
    {
        ;
    }
}

int isName(char *str)
{

    if (!isalpha(*str) && *str != '_')
    {
        return 0;
    }

    while (*++str)
    {
        if (!isalnum(*str) && *str != '_')
        {
            return 0;
        }
    }
    return 1;
}

size_t findClosingQuote(char *dat)
{
    char q = dat[0];
    if (q != '\'' && q != '"' && q != '`')
    {
        return 0;
    }

    size_t i = 0, length = strlen(dat);
    while (++i < length)
    {
        if (dat[i] == q)
        {
            if (dat[i - 1] == '\\')
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

size_t findClosingBrace(char *dat)
{
    char openingBrace = dat[0], closingBrace;
    if (openingBrace != '{' && openingBrace != '(')
    {
        return 0;
    }
    if (openingBrace == '{')
    {
        closingBrace = '}';
    }
    else
    {
        closingBrace = ')';
    }
    size_t obCount = 1, cbCount = 0;
    size_t i = 0, length = strlen(dat);
    while (++i < length)
    {
        if ((dat[i] == '"') || (dat[i] == '\'') || (dat[i] == '`'))
        {
            if (dat[i - 1] == '\\')
            {
                continue;
            }
            char q = dat[i];
            while (++i < length)
            {
                if (dat[i] == q && dat[i - 1] != '\\')
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
        if (dat[i - 1] != '\\')
        {
            if (dat[i] == openingBrace)
            {
                obCount++;
            }
            else if (dat[i] == closingBrace)
            {
                cbCount++;
            }
        }
        if (obCount == cbCount)
        {
            break;
        }
    }
    if (obCount != cbCount)
    {
        return 0;
    }
    return i;
}

char *substituteStr(char *temp1, char *temp2, size_t start, size_t end)
{

    char bef[start + 1];
    strncpy(bef, temp1, start);
    bef[start] = '\0';

    size_t afterlength = strlen(temp1) - end + 1;
    char after[afterlength];
    strcpy(after, temp1 + end + 1);

    size_t totallength = start + afterlength + strlen(temp2);
    char *fin = malloc(totallength + 1);
    if (!fin)
    {
        fprintf(stderr, "error: insufficient memory to perform variable substitution\n");
        return NULL;
    }
    if (!totallength)
    {
        fin[0] = '\0';
    }
    else
    {
        strcpy(fin, bef);
        strcat(fin, temp2);
        strcat(fin, after);
    }
    return fin;
}

int substituteWord(char **pstart, char **p, size_t length,
                   char *(func)(char *),
                   int addQuotes)
{
    char *temp = malloc(length + 1);
    if (!temp)
    {
        (*p) += length;
        return 0;
    }
    strncpy(temp, *p, length);
    temp[length--] = '\0';

    char *temp2;
    if (func)
    {
        temp2 = func(temp);
        if (temp2 == INVALID_VAR)
        {
            temp2 = NULL;
        }
        if (temp2)
        {
            free(temp);
        }
    }
    else
    {
        temp2 = temp;
    }

    if (!temp2)
    {
        (*p) += length;
        free(temp);
        return 0;
    }

    size_t i = (*p) - (*pstart);

    temp = quoteVal(temp2, addQuotes);
    free(temp2);
    if (temp)
    {
        if ((temp2 = substitute_str(*pstart, temp, i, i + length)))
        {
            free(*pstart);
            (*pstart) = temp2;
            length = strlen(temp);
        }
        free(temp);
    }

    (*p) = (*pstart) + i + length - 1;
    return 1;
}

struct wordS *wordExpand(char *origWord)
{
    if (!origWord)
    {
        return NULL;
    }

    if (!*origWord)
    {
        return makeWord(origWord);
    }

    char *pstart = malloc(strlen(origWord) + 1);
    if (!pstart)
    {
        return NULL;
    }
    strcpy(pstart, origWord);

    char *temp = pstart, *temp2;
    char *temp3;
    char c;
    size_t i = 0;
    size_t length;
    int inDoubleQuotes = 0;
    int inVarAssign = 0;
    int varAssignEq = 0;
    int expanded = 0;
    char *(*func)(char *);

    do
    {
        switch (*temp)
        {
        case '~':
            if (inDoubleQuotes)
            {
                break;
            }
            if (temp == pstart || (inVarAssign && (temp[-1] == ':' || (temp[-1] == '=' && varAssignEq == 1))))
            {
                int tildeQuoted = 0;
                int endme = 0;
                temp2 = temp + 1;

                while (*temp2)
                {
                    switch (*temp2)
                    {
                    case '\\':
                        tildeQuoted = 1;
                        temp2++;
                        break;

                    case '"':
                    case '\'':
                        i = findClosingQuote(temp2);
                        if (i)
                        {
                            tildeQuoted = 1;
                            temp2 += i;
                        }
                        break;

                    case '/':
                        endme = 1;
                        break;

                    case ':':
                        if (inVarAssign)
                        {
                            endme = 1;
                        }
                        break;
                    }
                    if (endme)
                    {
                        break;
                    }
                    temp2++;
                }
                if (tildeQuoted)
                {
                    temp = temp2;
                    break;
                }

                length = temp2 - temp;
                substituteWord(&pstart, &temp, length, tildeExpand, !inDoubleQuotes);
                expanded = 1;
            }
            break;

        case '"':
            inDoubleQuotes = !inDoubleQuotes;
            break;

        case '=':
            if (inDoubleQuotes)
            {
                break;
            }
            length = temp - pstart;
            temp3 = malloc(length + 1);

            if (!temp3)
            {
                fprintf(stderr, "error: insufficient memory for internal buffers\n");
                break;
            }

            strncpy(temp3, pstart, length);
            temp3[length] = '\0';

            if (isName(temp3))
            {
                inVarAssign = 1;
                varAssignEq++;
            }
            free(temp3);
            break;

        case '\\':
            temp++;
            break;

        case '\'':
            if (inDoubleQuotes)
            {
                break;
            }
            temp += findClosingQuote(temp);
            break;

        case '`':
            if ((length = findClosingQuote(temp)) == 0)
            {
                break;
            }
            substituteWord(&pstart, &temp, length + 1, commandSubstitute, 0);
            expanded = 1;
            break;
        case '$':
            c = temp[1];
            switch (c)
            {
            case '{':
                if ((length = findClosingBrace(temp + 1)) == 0)
                {
                    break;
                }
                if (!substituteWord(&pstart, &temp, length + 2, varExpand, 0))
                {
                    free(pstart);
                    return NULL;
                }

                expanded = 1;
                break;

            case '(':
                i = 0;

                if (temp[2] == '(')
                {
                    i++;
                }

                if ((length = findClosingBrace(temp + 1)) == 0)
                {
                    break;
                }
                func = i ? arithmExpand : commandSubstitute;
                substitute_word(&pstart, &temp, length + 2, func, 0);
                expanded = 1;
                break;

            default:
                if (!isalpha(temp[1]) && temp[1] != '_')
                {
                    break;
                }

                temp2 = temp + 1;

                while (*temp2)
                {
                    if (!isalnum(*temp2) && *temp2 != '_')
                    {
                        break;
                    }
                    temp2++;
                }
                if (temp2 == temp + 1)
                {
                    break;
                }

                substitute_word(&pstart, &temp, temp2 - temp, varExpand, 0);
                expanded = 1;
                break;
            }
            break;

        default:
            if (isspace(*temp) && !inDoubleQuotes)
            {
                expanded = 1;
            }
            break;
        }
    } while (*(++temp));

    struct wordS *words = NULL;
    if (expanded)
    {
        words = fieldSplit(pstart);
    }

    if (!words)
    {
        words = makeWord(pstart);
        if (!words)
        {
            fprintf(stderr, "error: insufficient memory\n");
            free(pstart);
            return NULL;
        }
    }
    free(pstart);

    words = pathnamesExpand(words);
    removeQuotes(words);

    return words;
}

char *tildeExpand(char *s)
{
    char *home = NULL;
    size_t length = strlen(s);
    char *s2 = NULL;
    struct symtabEntryS *ent;

    if (length == 1)
    {
        ent = getSymtabEntry("HOME");
        if (ent && ent->val)
        {
            home = ent->val;
        }
        else
        {
            struct passwd *pass;
            pass = getpwuid(getuid());
            if (pass)
            {
                home = pass->pw_dir;
            }
        }
    }
    else
    {
        struct passwd *pass;
        pass = getpwnam(s + 1);
        if (pass)
        {
            home = pass->pw_dir;
        }
    }

    if (!home)
    {
        return NULL;
    }

    s2 = malloc(strlen(home) + 1);
    if (!s2)
    {
        return NULL;
    }
    strcpy(s2, home);
    return s2;
}

char *varExpand(char *origVarName)
{
    if (!origVarName)
    {
        return NULL;
    }
    origVarName++;
    size_t length = strlen(origVarName);
    if (*origVarName == '{')
    {
        origVarName[length - 1] = '\0';
        origVarName++;
    }
    if (!*origVarName)
    {
        return NULL;
    }

    int getLength = 0;
    if (*origVarName == '#')
    {
        if (strchr(origVarName, ':'))
        {
            fprintf(stderr, "error: invalid variable substitution: %s\n", origVarName);
            return INVALID_VAR;
        }
        getLength = 1;
        origVarName++;
    }

    if (!*origVarName)
    {
        return NULL;
    }

    char *sub = strchr(origVarName, ':');
    if (!sub)
    {
        sub = strchrAny(origVarName, "-=?+%#");
    }

    length = sub ? (size_t)(sub - origVarName) : strlen(origVarName);

    if (sub && *sub == ':')
    {
        sub++;
    }

    char varName[length + 1];
    strncpy(varName, origVarName, length);
    varName[length] = '\0';

    char *emptyVal = "";
    char *temp = NULL;
    char set = 0;

    struct symtabEntryS *ent = getSymtabEntry(varName);
    temp = (ent && ent->val && ent->val[0]) ? ent->val : emptyVal;

    if (!temp || temp == emptyVal)
    {
        if (sub && *sub)
        {
            switch (sub[0])
            {
            case '-':
                temp = sub + 1;
                break;

            case '=':

                temp = sub + 1;

                set = 1;
                break;

            case '?':
                if (sub[1] == '\0')
                {
                    fprintf(stderr, "error: %s: parameter not set\n", varName);
                }
                else
                {
                    fprintf(stderr, "error: %s: %s\n", varName, sub + 1);
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
            temp = emptyVal;
        }
    }
    else
    {
        if (sub && *sub)
        {
            switch (sub[0])
            {
            case '-':
            case '=':
            case '?':
                break;

            case '+':
                temp = sub + 1;
                break;

            case '%':
                sub++;

                char *p = wordExpandToStr(temp);
                if (!p)
                {
                    return INVALID_VAR;
                }
                int longest = 0;
                if (*sub == '%')
                {
                    longest = 1, sub++;
                }
                if ((length = matchSuffix(sub, p, longest)) == 0)
                {
                    return p;
                }
                char *p2 = malloc(length + 1);
                if (p2)
                {
                    strncpy(p2, p, length);
                    p2[length] = '\0';
                }
                free(p);
                return p2;

            case '#':
                sub++;
                p = wordExpandToStr(temp);
                if (!p)
                {
                    return INVALID_VAR;
                }
                longest = 0;
                if (*sub == '#')
                {
                    longest = 1, sub++;
                }
                if ((length = matchPrefix(sub, p, longest)) == 0)
                {
                    return p;
                }
                p2 = malloc(strlen(p) - length + 1);
                if (p2)
                {
                    strcpy(p2, p + length);
                }
                free(p);
                return p2;

            default:
                return INVALID_VAR;
            }
        }
    }

    int expanded = 0;
    if (temp)
    {
        if ((temp = wordExpandToStr(temp)))
        {
            expanded = 1;
        }
    }

    if (set)
    {
        if (!ent)
        {
            ent = addToSymtab(varName);
        }
        if (ent)
        {
            symtabEntrySetval(ent, temp);
        }
    }

    char buffer[32];
    char *p = NULL;
    if (getLength)
    {
        if (!temp)
        {
            sprintf(buffer, "0");
        }
        else
        {
            sprintf(buffer, "%lu", strlen(temp));
        }
        p = malloc(strlen(buffer) + 1);
        if (p)
        {
            strcpy(p, buffer);
        }
    }
    else
    {
        p = malloc(strlen(temp) + 1);
        if (p)
        {
            strcpy(p, temp);
        }
    }

    if (expanded)
    {
        free(temp);
    }

    return p ?: INVALID_VAR;
}

char *commandSubstitute(char *origCmd)
{
    char br[1024];
    size_t bufferSize = 0;
    char *buffer = NULL;
    char *p = NULL;
    int i = 0;
    int backquoted = (*origCmd == '`');

    char *command = malloc(strlen(origCmd + 1));

    if (!command)
    {
        fprintf(stderr, "error: insufficient memory to perform command substitution\n");
        return NULL;
    }

    strcpy(command, origCmd + (backquoted ? 1 : 2));

    char *command2 = command;
    size_t commandLength = strlen(command);

    if (backquoted)
    {
        if (command[commandLength - 1] == '`')
        {
            command[commandLength - 1] = '\0';
        }

        char *p1 = command;

        do
        {
            if (*p1 == '\\' &&
                (p1[1] == '$' || p1[1] == '`' || p1[1] == '\\'))
            {
                char *p2 = p1, *p3 = p1 + 1;
                while ((*p2++ = *p3++))
                {
                    ;
                }
            }
        } while (*(++p1));
    }
    else
    {
        if (command[commandLength - 1] == ')')
        {
            command[commandLength - 1] = '\0';
        }
    }

    FILE *fp = popen(command2, "r");

    if (!fp)
    {
        free(command2);
        fprintf(stderr, "error: failed to open pipe: %s\n", strerror(errno));
        return NULL;
    }

    while ((i = fread(br, 1, 1024, fp)))
    {
        if (!buffer)
        {
            buffer = malloc(i + 1);
            if (!buffer)
            {
                goto fin;
            }

            p = buffer;
        }
        else
        {
            char *buffer2 = realloc(buffer, bufferSize + i + 1);

            if (!buffer2)
            {
                free(buffer);
                buffer = NULL;
                goto fin;
            }

            buffer = buffer2;
            p = buffer + bufferSize;
        }

        bufferSize += i;

        memcpy(p, br, i);
        p[i] = '\0';
    }

    if (!bufferSize)
    {
        free(command2);
        return NULL;
    }

    i = bufferSize - 1;

    while (buffer[i] == '\n' || buffer[i] == '\r')
    {
        buffer[i] = '\0';
        i--;
    }

fin:
    pclose(fp);

    free(command2);

    if (!buffer)
    {
        fprintf(stderr, "error: insufficient memory to perform command substitution\n");
    }

    return buffer;
}

static inline int isIFSChar(char c, char *IFS)
{
    if (!*IFS)
    {
        return 0;
    }

    do
    {
        if (c == *IFS)
        {
            return 1;
        }
    } while (*++IFS);

    return 0;
}

void skipIFSWhitespace(char **str, char *IFS)
{
    char *IFS2 = IFS;
    char *s2 = *str;

    do
    {
        if (*s2 == *IFS2)
        {
            s2++;
            IFS2 = IFS - 1;
        }
    } while (*++IFS2);

    *str = s2;
}

void skipIFSDelim(char *str, char *IFSSpace, char *IFSDelim, size_t *_i, size_t length)
{
    size_t i = *_i;

    while ((i < length) && isIFSChar(str[i], IFSSpace))
    {
        i++;
    }

    while ((i < length) && isIFSChar(str[i], IFSDelim))
    {
        i++;
    }

    while ((i < length) && isIFSChar(str[i], IFSSpace))
    {
        i++;
    }

    *_i = i;
}

struct wordS *fieldSplit(char *str)
{
    struct symtabEntryS *ent = getSymtabEntry("IFS");
    char *IFS = ent ? ent->val : NULL;
    char *p;

    if (!IFS)
    {
        IFS = " \t\n";
    }

    if (IFS[0] == '\0')
    {
        return NULL;
    }

    char IFSSpace[64];
    char IFSDelim[64];

    if (strcmp(IFS, " \t\n") == 0)
    {
        IFSSpace[0] = ' ';
        IFSSpace[1] = '\t';
        IFSSpace[2] = '\n';
        IFSSpace[3] = '\0';
        IFSDelim[0] = '\0';
    }
    else
    {
        p = IFS;
        char *sp = IFSSpace;
        char *dp = IFSDelim;

        do
        {
            if (isspace(*p))
            {
                *sp++ = *p++;
            }
            else
            {
                *dp++ = *p++;
            }
        } while (*p);

        *sp = '\0';
        *dp = '\0';
    }

    size_t length = strlen(str);
    size_t i = 0, j = 0, k;
    int fields = 1;
    char quote = 0;

    skipIFSWhitespace(&str, IFSSpace);

    do
    {
        switch (str[i])
        {
        case '\\':
            if (quote != '\'')
            {
                i++;
            }
            break;

        case '\'':
        case '"':
        case '`':
            if (quote == str[i])
            {
                quote = 0;
            }
            else
            {
                quote = str[i];
            }
            break;

        default:
            if (quote)
            {
                break;
            }

            if (isIFSChar(str[i], IFSSpace) || isIFSChar(str[i], IFSDelim))
            {
                skipIFSDelim(str, IFSSpace, IFSDelim, &i, length);
                if (i < length)
                {
                    fields++;
                }
            }
            break;
        }
    } while (++i < length);

    if (fields == 1)
    {
        return NULL;
    }

    struct wordS *firstField = NULL;
    struct wordS *cursor = NULL;

    i = 0;
    j = 0;
    quote = 0;

    do
    {
        switch (str[i])
        {
        case '\\':
            if (quote != '\'')
            {
                i++;
            }
            break;

        case '\'':
            p = str + i + 1;
            while (*p && *p != '\'')
            {
                p++;
            }
            i = p - str;
            break;

        case '"':
        case '`':
            if (quote == str[i])
            {
                quote = 0;
            }
            else
            {
                quote = str[i];
            }
            break;

        default:
            if (quote)
            {
                break;
            }

            if (isIFSChar(str[i], IFSSpace) ||
                isIFSChar(str[i], IFSDelim) || (i == length))
            {
                char *temp = malloc(i - j + 1);

                if (!temp)
                {
                    fprintf(stderr, "error: insufficient memory for field splitting\n");
                    return firstField;
                }

                strncpy(temp, str + j, i - j);
                temp[i - j] = '\0';

                struct wordS *field = malloc(sizeof(struct wordS));

                if (!field)
                {
                    free(temp);
                    return firstField;
                }

                field->data = temp;
                field->length = i - j;
                field->next = NULL;

                if (!firstField)
                {
                    firstField = field;
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

                k = i;

                skipIFSDelim(str, IFSSpace, IFSDelim, &i, length);
                j = i;

                if (i != k && i < length)
                {
                    i--;
                }
            }
            break;
        }
    } while (++i <= length);

    return firstField;
}

struct wordS *pathnamesExpand(struct wordS *words)
{
    struct wordS *w = words;
    struct wordS *pw = NULL;

    while (w)
    {
        char *temp = w->data;

        if (!hasGlobChars(temp, strlen(temp)))
        {
            pw = w;
            w = w->next;
            continue;
        }

        glob_t glob;
        char **matches = getFilenameMatches(temp, &glob);

        if (!matches || !matches[0])
        {
            globfree(&glob);
        }
        else
        {
            struct wordS *head = NULL, *tail = NULL;

            for (size_t j = 0; j < glob.gl_pathc; j++)
            {
                if (matches[j][0] == '.' &&
                    (matches[j][1] == '.' || matches[j][1] == '\0' || matches[j][1] == '/'))
                {
                    continue;
                }

                if (!head)
                {
                    head = makeWord(matches[j]);
                    tail = head;
                }
                else
                {
                    tail->next = makeWord(matches[j]);

                    if (tail->next)
                    {
                        tail = tail->next;
                    }
                }
            }

            if (w == words)
            {
                words = head;
            }
            else if (pw)
            {
                pw->next = head;
            }

            pw = tail;
            tail->next = w->next;

            w->next = NULL;
            freeAllWords(w);
            w = tail;

            globfree(&glob);
        }

        pw = w;
        w = w->next;
    }

    return words;
}

void removeQuotes(struct wordS *wordlist)
{
    if (!wordlist)
    {
        return;
    }

    int inDoubleQuotes = 0;
    struct wordS *word = wordlist;
    char *temp;

    while (word)
    {
        temp = word->data;
        while (*temp)
        {
            switch (*temp)
            {
            case '"':
                /* toggle quote mode */
                inDoubleQuotes = !inDoubleQuotes;
                deleteCharAt(temp, 0);
                break;

            case '\'':
                if (inDoubleQuotes)
                {
                    temp++;
                    break;
                }

                deleteCharAt(temp, 0);

                while (*temp && *temp != '\'')
                {
                    temp++;
                }

                if (*temp == '\'')
                {
                    deleteCharAt(temp, 0);
                }
                break;

            case '`':
                deleteCharAt(temp, 0);
                break;

            case '\v':
            case '\f':
            case '\t':
            case '\r':
            case '\n':
                temp++;
                break;

            case '\\':
                if (inDoubleQuotes)
                {
                    switch (temp[1])
                    {
                    case '$':
                    case '`':
                    case '"':
                    case '\\':
                    case '\n':
                        deleteCharAt(temp, 0);
                        temp++;
                        break;

                    default:
                        temp++;
                        break;
                    }
                }
                else
                {
                    deleteCharAt(temp, 0);
                    temp++;
                }
                break;

            default:
                temp++;
                break;
            }
        }

        word->length = strlen(word->data);

        word = word->next;
    }
}

char *wordExpandToStr(char *word)
{
    struct wordS *w = wordExpand(word);

    if (!w)
    {
        return NULL;
    }

    char *res = wordlistToStr(w);
    freeAllWords(w);

    return res;
}