#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
    ----------------------------------------------
    HOW TO COMPILE:
    1) Open terminal in folder with main.c
    2) Enter 'gcc -o lsh main.c' without quotes

    HOW TO RUN:
    1) After compiled, run './lsh' without quotes
    ----------------------------------------------
*/



// Shell commands
int cdlsh(char **args);
int helplsh(char **args);
int exitlsh(char **args);



// Built in commands
char *builtinStrings[] = 
{
    "cd",
    "help",
    "exit"
    // Where we could create the string values of new commands
};
int(*builtinFunctions[]) (char **) =
{
    &cdlsh,
    &helplsh,
    &exitlsh
    // Prototypes (I think) of all of the possible command functions
};
int builtinlshNum()
{
    return sizeof(builtinStrings) / sizeof(char *);
    // Size of array of functions
}



// Built in function implementation
int cdlsh(char **args) // Used to change directory
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "lsh: Expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}
int helplsh(char **args) // Used to explain commands
{
    int temp;

    printf("Group Final Assignment Shell\n");
    printf("Enter program names and arguments then press enter.\n");
    printf("Built in commands: \n");


    for (temp = 0; temp < builtinlshNum(); temp++)
    {
        printf(" %s\n", builtinStrings[temp]);
    }


    printf("Use man command for info on other programs.\n");
    return 1;
}
int exitlsh(char **args) // Used to quit shell
{
    return 0;
}
int launchlsh(char **args)
// Called in 'executelsh' to launch shell
{
    pid_t var;
    int sts;

    var = fork();
    if (var == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (var < 0)
    {
        perror("lsh");
    }
    else
    {
        do
        {
            waitpid(var, &sts, WUNTRACED);
        }
        while (!WIFEXITED(sts) && !WIFSIGNALED(sts));
    }

    return 1;
}
int executelsh(char **args)
// Called in looplsh
{
    int temp;

    if (args[0] == NULL)
    {
        return 1;
    }

    for (temp = 0; temp < builtinlshNum(); temp++)
    {
        if (strcmp(args[0], builtinStrings[temp]) == 0)
        {
            return (*builtinFunctions[temp])(args);
        }
    }

    return launchlsh(args);
}



// Reading line from stdin
char *readlinelsh(void)
{
#ifdef USE_STD_GET_LSH

    char *seg = NULL;
    ssize_t bufferSize = 0;

    if (getline(&seg, &bufferSize, stdin) == -1)
    {
        if (feof(stdin))
        {
            exit(EXIT_SUCCESS);
        }
        else{
            perror("lsh: getline\n");
            exit(EXIT_FAILURE)l;
        }
    }
    return seg;
#else
#define BUFFERSIZE_READLINE_LSH 1024

    int bufferSize = BUFFERSIZE_READLINE_LSH;
    int pos = 0;
    char *buffer = malloc(sizeof(char) * bufferSize);
    int a;

    if (!buffer)
    {
        fprintf(stderr, "lsh: error with allocation\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        a = getchar();
        if (a == EOF)
        {
            exit(EXIT_SUCCESS);
        }
        else if (a == '\n')
        {
            buffer[pos] = '\0';
            return buffer;
        }
        else
        {
            buffer[pos] = a;
        }

        pos++;

        if (pos >= bufferSize)
        {
            bufferSize += BUFFERSIZE_READLINE_LSH;
            buffer = realloc(buffer, bufferSize);

            if(!buffer)
            {
                fprintf(stderr, "lsh: error with allocation\n");
                exit(EXIT_FAILURE);
            }
        }
    }
#endif
}



#define BUFFERSIZE_TOKEN_LSH 64
#define DELIMITER_TOKEN_LSH " \t\r\n\a"


char **splitlinelsh(char *seg)
{
    int bufferSize = BUFFERSIZE_TOKEN_LSH;
    int pos = 0;
    char **tokens = malloc(bufferSize * sizeof(char*));
    char *token;
    char **backupTokens;


    if (!tokens)
    {
        fprintf(stderr, "lsh: error with allocation\n");
        exit(EXIT_FAILURE);
    }


    token = strtok(seg, DELIMITER_TOKEN_LSH);
    while (token!= NULL)
    {
        tokens[pos] = token;
        pos++;


        if (pos >= bufferSize)
        {
            bufferSize += BUFFERSIZE_TOKEN_LSH;
            backupTokens = tokens;
            tokens = realloc(tokens, bufferSize * sizeof(char*));
            if (!tokens)
            {
                free(backupTokens);
                fprintf(stderr, "lsh: error with allocation\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIMITER_TOKEN_LSH);
    }
    tokens[pos] = NULL;
    return tokens;
}


void looplsh(void)
{
    char *seg;
    char **args;
    int sts;


    do
    {
        printf("> ");
        seg = readlinelsh();
        args = splitlinelsh(seg);
        sts = executelsh(args);
        free(seg);
        free(args);
    }
    while (sts);
}



int main(int argc, char **argv)
{
    // Launching shell, waits for command/exit of shell
    looplsh();

    return EXIT_SUCCESS;
}