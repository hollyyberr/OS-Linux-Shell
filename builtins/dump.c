#include "../shell.h"
#include "../symtab/symtab.h"

// Implements dump utility
int dump(int argc, char **argv)
{
    dumpLocalSymtab();
    return 0;
}