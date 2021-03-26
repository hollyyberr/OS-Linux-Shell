#include <unistd.h>
#include "shell.h"
#include "parser.h"
#include "scanner.h"
#include "node.h"
#include "source.h"

struct nodeS *parseSimpleCommand(struct tokenS *token) {

    if(!token) {
        return NULL;
    }

    struct nodeS *command = newNode(NODE_COMMAND);

    if(!command) {
        freeToken(token);
        return NULL;
    }

    struct sourceS *src = token->src;

    do {
        if(token->text[0] == '\n') {
            freeToken(token);
            break;
        }
        struct nodeS *word = newNode(NODE_VAR);
        if(!word) {
            freeNodeTree(command);
            freeToken(token);
            return NULL;
        }
        setNodeValStr(word, token->text);
        addChildNode(command, word);
        freeToken(token);
    } while((token = tokenize(src)) != &eofToken);

    return command;
}