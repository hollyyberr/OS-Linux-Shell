#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <regex.h>
#include <fnmatch.h>
#include <locale.h>
#include <glob.h>
#include <sys/stat.h>
#include "shell.h"

int hasGlobChars(char *a, size_t length)
{
    char *b = a + length;
    char openBrac = 0, closeBrac = 0;
    while (a < b && *a)
    {
        switch (*a)
        {
        case '*':
        case '?':
            return 1;

        case '[':
            openBrac++;
            break;

        case ']':
            closeBrac++;
            break;
        }
        a++;
    }

    if (openBrac && openBrac == closeBrac)
    {
        return 1;
    }
    return 0;
}

int matchPrefix(char *ptrn, char *seg, int lng)
{
    if (!ptrn || !seg)
    {
        return 0;
    }
    char *a = seg + 1;
    char b = *a;
    char *aMatch = NULL;
    char *lMatch = NULL;
    while (b)
    {
        *a = '\0';
        if (fnmatch(ptrn, seg, 0) == 0)
        {
            if (!aMatch)
            {
                if (!lng)
                {
                    *a = b;
                    return a - seg;
                }
                aMatch = a;
            }
            lMatch = a;
        }
        *a = b;
        b = *(++a);
    }
    /* check the result of the comparison */
    if (lMatch)
    {
        return lMatch - seg;
    }
    if (aMatch)
    {
        return aMatch - seg;
    }
    return 0;
}

int matchSuffix(char *ptrn, char *a, int lng)
{
    if (!ptrn || !a)
    {
        return 0;
    }
    char *seg = a + strlen(a) - 1;
    char *aMatch = NULL;
    char *lMatch = NULL;
    while (seg > a)
    {
        if (fnmatch(ptrn, a, 0) == 0)
        {
            if (!aMatch)
            {
                if (!lng)
                {
                    return seg - a;
                }
                aMatch = seg;
            }
            lMatch = seg;
        }
        seg--;
    }
    if (lMatch)
    {
        return lMatch - a;
    }
    if (aMatch)
    {
        return aMatch - a;
    }
    return 0;
}

char **getFilenameMatches(char *ptrn, glob_t *matches)
{
    matches->gl_pathc = 0;
    matches->gl_pathv = NULL;

    if (!ptrn)
    {
        return NULL;
    }

    int result = glob(ptrn, 0, NULL, matches);

    if (result != 0)
    {
        globfree(matches);
        return NULL;
    }

    return matches->gl_pathv;
}