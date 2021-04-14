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
    2) Enter 'gcc -o shell main.c' without quotes

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
int run_task_com(char **args);



// Built in commands
char *builtinStrings[] = 
{
    "cd",
    "help",
    "exit",
    "ps",
    "history"
    "run"
    // Where we could create the string values of new commands
};
int(*builtinFunctions[]) (char **) =
{
    &cd_com,
    &help_com,
    &exit_com,
    &ps_com,
    &history_com,
    &run_task_com
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
            perror("lsh");
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
int run_task_com(char **args)
// Used to launch new task foreground/background
{
    int size = sizeof args / sizeof *args;
    char *amp = "&";
    if (args[size-1] == "&")
    {
        char *command = (char *) malloc(1 + strlen(args[1]) + strlen(amp));
        strcpy(command, args[1]);
        strcat(command, amp);
        system(command);
        printf("Running job in background");
    }
    else
    {
        system(args[1]);
        printf("Running job in foreground");
    }
    return 1;
}
int exit_com(char **args) 
// Used to quit shell
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
        fprintf(stderr, "Shell: error with allocation\n");
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