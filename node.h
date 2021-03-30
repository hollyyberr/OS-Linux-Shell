#ifndef NODE_H
#define NODE_H

enum nodeTypeE
{
    NODE_COMMAND,
    NODE_VAR,
};

enum valTypeE
{
    VAL_SINT = 1,
    VAL_UINT,
    VAL_SLLONG,
    VAL_ULLONG,
    VAL_FLOAT,
    VAL_LDOUBLE,
    VAL_CHR,
    VAL_STR,
};

union symvalU
{
    long sint;
    unsigned long uint;
    long long sllong;
    unsigned long long ullong;
    double sfloat;
    long double ldouble;
    char chr;
    char *str;
};

struct nodeS
{
    enum nodeTypeE type;
    enum valTypeE valType;
    union symvalU val;
    int children;
    struct nodeS *firstChild;
    struct nodeS *nextSibling, *prevSibling;
};

struct nodeS *newNode(enum nodeTypeE type);
void addChildNode(struct nodeS *parent, struct nodeS *child);
void freeNodeTree(struct nodeS *node);
void setNodeValStr(struct nodeS *node, char *val);

#endif