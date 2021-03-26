#ifndef BACKEND_H
#define BACKEND_H

#include "node.h"

char *searchPath(char *file);
int doExecCommand(int argc, char **argv);
int doSimpleCommand(struct nodeS *node);

#endif