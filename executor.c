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

char *searchPath(char *file) {
    char *PATH = getenv("PATH");
    char *p1 = PATH;
    char *p2;

    while(p1 && *p1) {
        p2 = p1;

        while(*p2 && *p2 != ':') {
            p2++;
        }

        int pLength = p2-p1;
        if(!pLength) {
            pLength = 1;
        }

        int aLength = strlen(file);
        char path[pLength + 1 + aLength + 1];

        strncpy(path, p1, p2-p1);
        path[p2-p1] = '\0';

        if(p2[-1] != '/') {
            strcat(path, "/");
        }

        strcat(path, file);

        struct stat st;
        if(stat(path, &st) == 0) {
            if(!S_ISREG(st.st_mode)) {
                errno = ENOENT;
                p1 = p2;
                if(*p2 == ':') {
                    p1++;
                }
                continue;
            }

            p1 = malloc(strlen(path) + 1);
            if(!p1) {
                return NULL;
            }

            strcpy(p1, path);
            return p1;
        }
        else {
            p1 = p2;
            if(*p2 == ':') {
                p1++;
            }
        }
    }

    errno = ENOENT;
    return NULL;
}

int doExecCommand(int argc, char **argv) {
    if(strchr(argv[0], '/')) {
        execv(argv[0], argv);
    }
    else {
        char *path = searchPath(argv[0]);
        if(!path) {
            return 0;
        }
        execv(path, argv);
        free(path);
    }
    return 0;
}

static inline void freeArgv(int argc, char **argv) {
    if(!argc) {
        return;
    }

    while(argc--) {
        free(argv[argc]);
    }
}

int doSimpleCommand(struct nodeS *node) {
    if(!node) {
        return 0;
    }

    struct nodeS *child = node->firstChild;
    if(!child) {
        return 0;
    }
    int argc = 0;
    long maxArgs = 255;
    char *argv[maxArgs+1];
    char *str;

    while(child) {
        str = child->val.str;
        argv[argc] = malloc(strlen(str) + 1);

        if(!argv[argc]) {
            freeArgv(argc, argv);
            return 0;
        }

        strcpy(argv[argc], str);
        if(++argc >= maxArgs) {
            break;
        }

        child = child->nextSibling;
    }

    argv[argc] = NULL;

    pid_t childPid = 0;
    if((childPid = fork()) == 0) {
        doExecCommand(argc, argv);
        fprintf(stderr, "Error: could not execute command: %s\n", strerror(errno));
        if(errno == ENOEXEC) {
            exit(126);
        }
        else if (errno == ENOENT) {
            exit(127);
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    else if(childPid < 0) {
        fprintf(stderr, "Error: Could not fork command %s\n", strerror(errno));
        return 0;
    }

    int sts = 0;
    waitpid(childPid, &sts, 0);
    freeArgv(argc, argv);

    return 1;
}