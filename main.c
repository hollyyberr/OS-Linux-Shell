#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

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
int cd_com(char **args);
int help_com(char **args);
int exit_com(char **args);
int ps_com(char **args);



// Built in commands
char *builtinStrings[] = 
{
    "cd",
    "help",
    "exit",
    "ps"
    // Where we could create the string values of new commands
};
int(*builtinFunctions[]) (char **) =
{
    &cd_com,
    &help_com,
    &exit_com,
    &ps_com
    // Prototypes (I think) of all of the possible command functions
};
int builtinNum()
{
    return sizeof(builtinStrings) / sizeof(char *);
    // Size of array of functions
}



// Built in function implementation
int cd_com(char **args) // Used to change directory
{
    if (args[1] == NULL)
    {
        char *homeDir;
        struct passwd *home;
        home = getpwuid(getuid());
        homeDir = home->pw_dir;
        if(chdir(homeDir) != 0) {
            perror("lsh");
        }
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
int help_com(char **args) // Used to explain commands
{
    int temp;

    printf("Group Final Assignment Shell\n");
    printf("Enter program names and arguments then press enter.\n");
    printf("Built in commands: \n");


    for (temp = 0; temp < builtinNum(); temp++)
    {
        printf(" %s\n", builtinStrings[temp]);
    }

    printf(" ls\n");

    return 1;
}
int ps_com(char **args)
{
    printf("CURRENT RUNNING PROCESSES\n");
    system("ps -e");
}
int exit_com(char **args) // Used to quit shell
{
    return 0;
}
int launch_com(char **args)
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
int execute_com(char **args)
// Called in looplsh
{
    int temp;

    if (args[0] == NULL)
    {
        return 1;
    }

    for (temp = 0; temp < builtinNum(); temp++)
    {
        if (strcmp(args[0], builtinStrings[temp]) == 0)
        {
            return (*builtinFunctions[temp])(args);
        }
    }

    return launch_com(args);
}



// Reading line from stdin
char *readline_com(void)
{
#ifdef USE_STD_GET

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
#define BUFFERSIZE_READLINE 1024

    int bufferSize = BUFFERSIZE_READLINE;
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
            bufferSize += BUFFERSIZE_READLINE;
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



#define BUFFERSIZE_TOKEN 64
#define DELIMITER_TOKEN " \t\r\n\a"


char **splitline_com(char *seg)
{
    int bufferSize = BUFFERSIZE_TOKEN;
    int pos = 0;
    char **tokens = malloc(bufferSize * sizeof(char*));
    char *token;
    char **backupTokens;


    if (!tokens)
    {
        fprintf(stderr, "lsh: error with allocation\n");
        exit(EXIT_FAILURE);
    }


    token = strtok(seg, DELIMITER_TOKEN);
    while (token!= NULL)
    {
        tokens[pos] = token;
        pos++;


        if (pos >= bufferSize)
        {
            bufferSize += BUFFERSIZE_TOKEN;
            backupTokens = tokens;
            tokens = realloc(tokens, bufferSize * sizeof(char*));
            if (!tokens)
            {
                free(backupTokens);
                fprintf(stderr, "lsh: error with allocation\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIMITER_TOKEN);
    }
    tokens[pos] = NULL;
    return tokens;
}


void loop_com(void)
{
    char *seg;
    char **args;
    int sts;


    do
    {
        char pathName[4096];
        if(getcwd(pathName, sizeof(pathName)) != NULL) {
            printf("%s", pathName);
        }
        else {
            perror("getcwd() error");
        }

        printf("> ");
        seg = readline_com();
        args = splitline_com(seg);
        sts = execute_com(args);
        free(seg);
        free(args);
    }
    while (sts);
}



int main(int argc, char **argv)
{
    // Launching shell, waits for command/exit of shell
    loop_com();

    return EXIT_SUCCESS;
}