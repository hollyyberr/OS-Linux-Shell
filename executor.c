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
    char *a = PATH;
    char *b;
    
    while(a && *a)
    {
        b = a;

        while(*b && *b != ':')
        {
            b++;
        }
        
	int  pLength = b-a;
        if(!pLength)
        {
            pLength = 1;
        }
        
        int  aLength = strlen(file);
        char path[pLength+1+aLength+1];
        
	strncpy(path, a, b-a);
        path[b-a] = '\0';
        
	if(b[-1] != '/')
        {
            strcat(path, "/");
        }

        strcat(path, file);
        
	struct stat temp;
        if(stat(path, &temp) == 0)
        {
            if(!S_ISREG(temp.st_mode))
            {
                errno = ENOENT;
                a = b;
                if(*b == ':')
                {
                    a++;
                }
                continue;
            }

            a = malloc(strlen(path)+1);
            if(!a)
            {
                return NULL;
            }
            
	    strcpy(a, path);
            return a;
        }
        else
        {
            // When file is not found
            a = b;
            if(*b == ':')
            {
                a++;
            }
        }
    }

    errno = ENOENT;
    return NULL;
}


int doExecCommand(int c, char **v)
{
    if(strchr(v[0], '/'))
    {
        execv(v[0], v);
    }
    else
    {
        char *path = searchPath(v[0]);
        if(!path)
        {
            return 0;
        }
        execv(path, v);
        free(path);
    }
    return 0;
}


static inline void freeArgv(int c, char **v)
{
    if(!c)
    {
        return;
    }

    while(c--)
    {
        free(v[c]);
    }
}


int doSimpleCommand(struct nodeS *node)
{
    if(!node)
    {
        return 0;
    }

    struct nodeS *child = node->firstChild;
    if(!child)
    {
        return 0;
    }
    
    int argCount = 0;
    int totalargCount = 0;
    char **argv = NULL;
    char *str;

    while(child)
    {
        str = child->val.str;
        struct wordS *word = wordExpand(str);
        

        if(!word)
        {
            child = child->nextSibling;
            continue;
        }

        struct wordS *word2 = word;
        while(word2)
        {
            if(checkBufferBounds(&argCount, &totalargCount, &argv))
            {
                str = malloc(strlen(word2->data)+1);
                if(str)
                {
                    strcpy(str, word2->data);
                    argv[argCount++] = str;
                }
            }
            word2 = word2->next;
        }
        
        freeAllWords(word);
        
        child = child->nextSibling;
    }

    if(checkBufferBounds(&argCount, &totalargCount, &argv))
    {
        argv[argCount] = NULL;
    }

    int i = 0;
    for( ; i < builtinsCount; i++)
    {
        if(strcmp(argv[0], builtins[i].name) == 0)
        {
            builtins[i].func(argCount, argv);
            freeBuffer(argCount, argv);
            return 1;
        }
    }

    pid_t childPid = 0;
    if((childPid = fork()) == 0)
    {
        doExecCommand(argCount, argv);
        fprintf(stderr, "Error: Could not execute command: %s\n", strerror(errno));
        if(errno == ENOEXEC)
        {
            exit(126);
        }
        else if(errno == ENOENT)
        {
            exit(127);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else if(childPid < 0)
    {
        fprintf(stderr, "Error: Could not fork command: %s\n", strerror(errno));
	    freeBuffer(argCount, argv);
        return 0;
    }

    int sts = 0;
    waitpid(childPid, &sts, 0);
    freeBuffer(argCount, argv);
    
    return 1;
}