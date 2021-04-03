#include <string.h>
#include "shell.h"
#include "symtab/symtab.h"

extern char **environ;

void initsh(void)
{
    initSymtab();

    struct symtabEntryS *ent;
    char **env = environ;

    while(*env)
    {
        char *temp = strchr(*env, '=');
        if(temp)
        {
            int length = temp-(*env);
            char name[length+1];

            strncpy(name, *env, length);
            name[length] = '\0';
            ent = addToSymtab(name);

            if(ent)
            {
                symtabEntrySetval(ent, temp+1);
                ent->flags |= FLAG_EXPORT;
            }
        }
        else
        {
            ent = addToSymtab(*env);
        }
        env++;
    }

    ent = addToSymtab("PS1");
    symtabEntrySetval(ent, "$ ");

    ent = addToSymtab("PS2");
    symtabEntrySetval(ent, "> ");
}