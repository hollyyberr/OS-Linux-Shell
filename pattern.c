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

int hasGlobChars(char *temp, sizeT length)
{
    char *temp2 = temp + length;
    char openBrac = 0, closBrac = 0;
    while (temp < temp2 && *temp)
    {
        switch (*temp)
        {
        case '*':
        case '?':
            return 1;

        case '[':
            openBrac++;
            break;

        case ']':
            closBrac++;
            break;
        }
        temp++;
    }
    if (openBrac && openBrac == closBrac)
    {
        return 1;
    }
    return 0;
}

int matchPrefix(char *ptrn, char *str, int longest)
{
    if (!ptrn || !str)
    {
        return 0;
    }
    char *s = str + 1;
    char c = *s;
    char *sMatch = NULL;
    char *lMatch = NULL;
    while (c)
    {
        *s = '\0';
        if (fnmatch(ptrn, str, 0) == 0)
        {
            if (!sMatch)
            {
                if (!longest)
                {
                    *s = c;
                    return s - str;
                }
                sMatch = s;
            }
            lMatch = s;
        }
        *s = c;
        c = *(++s);
    }
    if (lMatch)
    {
        return lMatch - str;
    }
    if (sMatch)
    {
        return sMatch - str;
    }
    return 0;
}

int matchSuffix(char *ptrn, char *str, int longest)
{
    if (!ptrn || !str)
    {
        return 0;
    }
    char *s = str + strlen(str) - 1;
    char *sMatch = NULL;
    char *lMatch = NULL;
    while (s > str)
    {
        if (fnmatch(ptrn, str, 0) == 0)
        {
            if (!sMatch)
            {
                if (!longest)
                {
                    return s - str;
                }
                sMatch = s;
            }
            lMatch = s;
        }
        s--;
    }
    if (lMatch)
    {
        return lMatch - str;
    }
    if (sMatch)
    {
        return sMatch - str;
    }
    return 0;
}

char **getFilenameMatches(char *ptrn, globT *matches)
{

    matches->glPathc = 0;
    matches->glPathv = NULL;

    if (!ptrn)
    {
        return NULL;
    }

    int res = glob(ptrn, 0, NULL, matches);

    /* return the result */
    if (res != 0)
    {
        globfree(matches);
        return NULL;
    }

    return matches->glPathv;
}