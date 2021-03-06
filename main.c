#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <ncurses.h>

/*
    ----------------------------------------------
    HOW TO COMPILE:
    1) Open terminal in folder with main.c
    2) Enter 'gcc -o shell main.c -lreadline' without quotes

    HOW TO RUN:
    1) After compiled, run './shell' without quotes
    ----------------------------------------------
*/

// Shell commands
int cd_com(char **args);
int help_com(char **args);
int exit_com(char **args);
int ps_com(char **args);
int history_com(char **args);
int jobs_com(char **args);



// Built in commands
char *builtinStrings[] = 
{
    "cd",
    "help",
    "exit",
    "ps",
    "history",
    "jobs"
    // Where we could create the string values of new commands
};
int(*builtinFunctions[]) (char **) =
{
    &cd_com,
    &help_com,
    &exit_com,
    &ps_com,
    &history_com,
    &jobs_com
    // Prototypes (I think) of all of the possible command functions
};
int builtinNum()
{
    return sizeof(builtinStrings) / sizeof(char *);
    // Size of array of functions
}

int history_com(char **args) {
    int l;
    HIST_ENTRY** list = history_list();
    if(list) {
        for(l=0;list[l];l++) {
            printf("%s\n",list[l]->line);
        }
    }
    return 1;
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
            perror("Shell");
        }
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("Shell");
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

    printf(" Shell\n");

    return 1;
}
int ps_com(char **args)
// Used to print all currently running processes
{
    printf("CURRENT RUNNING PROCESSES\n");
    system("ps -e");
    return 1;
}
int jobs_com(char **args)
// Used to print currently running jobs
{
    system("jobs");
    return 1;
}
int exit_com(char **args) 
// Used to quit shell
{
    return 0;
}
int launch_com(char **args)
// Called in 'execute_com' to launch process
{
    pid_t var;
    int sts;

    var = fork();
    if (var == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("Shell");
        }
        exit(EXIT_FAILURE);
    }
    else if (var < 0)
    {
        perror("Shell");
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
// Called in main method
{
    int temp;

    // Checks if nothing has been entered
    // Prints another Shell prompt
    if (args[0] == NULL)
    {
        return 1;
    }

    // Runs through array of all possible commands
    // If user enters builtin command, executes
    for (temp = 0; temp < builtinNum(); temp++)
    {
        if (strcmp(args[0], builtinStrings[temp]) == 0)
        {
            return (*builtinFunctions[temp])(args);
        }
    }

    // If user enters a command that is not builtin
    // Tries to launch command
    return launch_com(args);
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

    // If there are no tokens, Shell sends error and exits
    // Code after if statement does not execute
    if (!tokens)
    {
        fprintf(stderr, "Shell: error with allocation\n");
        exit(EXIT_FAILURE);
    }

    // Creates an array of tokens
    // Increases size of the array as needed
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
                fprintf(stderr, "Shell: error with allocation\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIMITER_TOKEN);
    }
    tokens[pos] = NULL;
    return tokens;
}

int main(int argc, char **argv)
{
    using_history();

    rl_bind_key('\t', rl_complete);


    // Launching shell, waits for command/exit of shell
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
        char input[100];
        input[0] = '\0';
        seg = readline(input);

        add_history(seg);

        args = splitline_com(seg);
        sts = execute_com(args);
        free(seg);
        free(args);
    }
    while (sts);

    return EXIT_SUCCESS;
}