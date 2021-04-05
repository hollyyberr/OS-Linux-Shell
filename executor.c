#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "shell.h"
#include "node.h"
#include "executor.h"

char *searchPath(char *file)
{
    char *PATH = getenv("PATH");
    char *p1 = PATH;
    char *p2;

    while (p1 && *p1)
    {
        p2 = p1;

        while (*p2 && *p2 != ':')
        {
            p2++;
        }

        int pLength = p2 - p1;
        if (!pLength)
        {
            pLength = 1;
        }

        int aLength = strlen(file);
        char path[pLength + 1 + aLength + 1];

        strncpy(path, p1, p2 - p1);
        path[p2 - p1] = '\0';

        if (p2[-1] != '/')
        {
            strcat(path, "/");
        }

        strcat(path, file);

        struct stat st;
        if (stat(path, &st) == 0)
        {
            if (!S_ISREG(st.st_mode))
            {
                errno = ENOENT;
                p1 = p2;
                if (*p2 == ':')
                {
                    p1++;
                }
                continue;
            }

            p1 = malloc(strlen(path) + 1);
            if (!p1)
            {
                return NULL;
            }

            strcpy(p1, path);
            return p1;
        }
        else /* file not found */
        {
            p1 = p2;
            if (*p2 == ':')
            {
                p1++;
            }
        }
    }

    errno = ENOENT;
    return NULL;
}

int doExecCommand(int argc, char **argv)
{
    if (strchr(argv[0], '/'))
    {
        execv(argv[0], argv);
    }
    else
    {
        char *path = searchPath(argv[0]);
        if (!path)
        {
            return 0;
        }
        execv(path, argv);
        free(path);
    }
    return 0;
}

static inline void freeArgv(int argc, char **argv)
{
    if (!argc)
    {
        return;
    }

    while (argc--)
    {
        free(argv[argc]);
    }
}

int doSimpleCommand(struct nodeS *node)
{
    if (!node)
    {
        return 0;
    }

    struct nodeS *child = node->firstChild;
    if (!child)
    {
        return 0;
    }

    int argc = 0;
    int tArgc = 0;
    char **argv = NULL;
    char *str;

    while (child)
    {
        str = child->val.str;
        struct wordS *w = wordExpand(str);

        if (!w)
        {
            child = child->nextSibling;
            continue;
        }

        struct wordS *w2 = w;
        while (w2)
        {
            if (checkBufferBounds(&argc, &tArgc, &argv))
            {
                str = malloc(strlen(w2->data) + 1);
                if (str)
                {
                    strcpy(str, w2->data);
                    argv[argc++] = str;
                }
            }
            w2 = w2->next;
        }

        freeAllWords(w);

        child = child->nextSibling;
    }

    if (checkBufferBounds(&argc, &tArgc, &argv))
    {
        argv[argc] = NULL;
    }
    int i = 0;
    for (; i < builtinsCount; i++)
    {

        if (strcmp(argv[0], builtins[i].name) == 0)
        {

            builtins[i].func(argc, argv);
            freeArgv(argc, argv);
            return 1;
        }
    }
    pid_t childPid = 0;
    if ((childPid = fork()) == 0)
    {
        doExecCommand(argc, argv);
        fprintf(stderr, "error: Could not execute command: %s\n", strerror(errno));
        if (errno == ENOEXEC)
        {
            exit(126);
        }
        else if (errno == ENOENT)
        {
            exit(127);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else if (childPid < 0)
    {
        fprintf(stderr, "error: Could not fork command: %s\n", strerror(errno));
        freeBuffer(argc, argv);
        return 0;
    }

    int sts = 0;
    waitpid(childPid, &sts, 0);
    freeBuffer(argc, argv);
    // freeArgv(argc, argv);

    return 1;
}