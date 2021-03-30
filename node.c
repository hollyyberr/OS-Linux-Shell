#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "shell.h"
#include "node.h"
#include "parser.h"

struct nodeS *newNode(enum nodeTypeE type)
{
    struct nodeS *node = malloc(sizeof(struct nodeS));

    if (!node)
    {
        return NULL;
    }

    memset(node, 0, sizeof(struct nodeS));
    node->type = type;

    return node;
}

void addChildNode(struct nodeS *parent, struct nodeS *child)
{
    if (!parent || !child)
    {
        return;
    }

    if (!parent->firstChild)
    {
        parent->firstChild = child;
    }
    else
    {
        struct nodeS *sibling = parent->firstChild;

        while (sibling->nextSibling)
        {
            sibling = sibling->nextSibling;
        }

        sibling->nextSibling = child;
        child->prevSibling = sibling;
    }
    parent->children++;
}

void setNodeValStr(struct nodeS *node, char *val)
{
    node->valType = VAL_STR;

    if (!val)
    {
        node->val.str = NULL;
    }
    else
    {
        char *val2 = malloc(strlen(val) + 1);

        if (!val2)
        {
            node->val.str = NULL;
        }
        else
        {
            strcpy(val2, val);
            node->val.str = val2;
        }
    }
}

void freeNodeTree(struct nodeS *node)
{
    if (!node)
    {
        return;
    }

    struct nodeS *child = node->firstChild;

    while (child)
    {
        struct nodeS *next = child->nextSibling;
        freeNodeTree(child);
        child = next;
    }

    if (node->valType == VAL_STR)
    {
        if (node->val.str)
        {
            free(node->val.str);
        }
    }
    free(node);
}