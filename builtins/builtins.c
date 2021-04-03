#include "../shell.h"

struct builtinS builtins[] =
{   
    {"dump", dump},
};

int builtinsCount = sizeof(builtins)/sizeof(struct builtinS);