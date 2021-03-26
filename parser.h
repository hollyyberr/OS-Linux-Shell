#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "source.h"

struct nodeS *parseSimpleCommand(struct tokenS *token);

#endif