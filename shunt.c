#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"
#include "symtab/symtab.h"

#define MAXOPSTACK 64
#define MAXNUMSTACK 64
#define MAXBASE 36

struct stackItemS
{
#define ITEM_LONG_INT 1
#define ITEM_VAR_PTR 2
    int type;
    union
    {
        long val;
        struct symtabEntryS *pointer;
    };
};

struct opS *opStack[MAXOPSTACK];
int nopStack = 0;

struct stackItemS numStack[MAXNUMSTACK];
int nNumStack = 0;
int err = 0;

long longValue(struct stackItemS *item)
{
    if (item->type == ITEM_LONG_INT)
    {
        return item->val;
    }
    else if (item->type == ITEM_VAR_PTR)
    {
        if (item->pointer->val)
        {
            return atol(item->pointer->val);
        }
    }
    return 0;
}

long evalUminus(struct stackItemS *item1, struct stackItemS *item2 __attribute__((unused)))
{
    return -long_value(item1);
}

long evalUplus(struct stackItemS *item1, struct stackItemS *item2 __attribute__((unused)))
{
    return long_value(item1);
}

long evalLognot(struct stackItemS *item1, struct stackItemS *item2 __attribute__((unused)))
{
    return !long_value(item1);
}

long evalBitnot(struct stackItemS *item1, struct stackItemS *item2 __attribute__((unused)))
{
    return ~long_value(item1);
}

long evalMult(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) * long_value(item2);
}

long evalAdd(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) + long_value(item2);
}

long evalSub(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) - long_value(item2);
}

long evalLsh(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) << long_value(item2);
}

long evalRsh(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) >> long_value(item2);
}

long evalLt(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) < long_value(item2);
}

long evalLe(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) <= long_value(item2);
}

long evalGt(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) > long_value(item2);
}

long evalGe(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) >= long_value(item2);
}

long evalEq(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) == long_value(item2);
}

long evalNe(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) != long_value(item2);
}

long evalBitand(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) & long_value(item2);
}

long evalBitxor(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) ^ long_value(item2);
}

long evalBitor(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) | long_value(item2);
}

long evalLogand(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) && long_value(item2);
}

long evalLogor(struct stackItemS *item1, struct stackItemS *item2)
{
    return long_value(item1) || long_value(item2);
}

long doEvalExp(long item1, long item2)
{
    return item2 < 0 ? 0 : (item2 == 0 ? 1 : item1 * doEvalExp(item1, item2 - 1));
}

long evalExp(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalExp(long_value(item1), long_value(item2));
}

long evalDiv(struct stackItemS *item1, struct stackItemS *item2)
{
    error = 0;
    long new2 = long_value(item2);
    if (!new2)
    {
        fprintf(stderr, "Error: Cannot divide by zero\n");
        error = 1;
        return 0;
    }
    return long_value(item1) / new2;
}

long evalMod(struct stackItemS *item1, struct stackItemS *item2)
{
    error = 0;
    long new2 = long_value(item2);
    if (!new2)
    {
        fprintf(stderr, "Error: Cannot divide by zero\n");
        error = 1;
        return 0;
    }
    return long_value(item1) % new2;
}

long evalAssign(struct stackItemS *item1, struct stackItemS *item2)
{
    long v = long_value(item2);
    if (item1->type == ITEM_VAR_PTR)
    {
        char buffer[16];
        sprintf(buffer, "%ld", v);
        symtab_entry_setval(item1->pointer, buffer);
    }
    return v;
}

long doEvalAssignExt(long (*f)(struct stackItemS *item1, struct stackItemS *item2),
                     struct stackItemS *item1, struct stackItemS *item2)
{
    long v = f(item1, item2);
    if (item1->type == ITEM_VAR_PTR)
    {
        char buffer[16];
        sprintf(buffer, "%ld", v);
        symtabEntrySetval(item1->pointer, buffer);
    }
    return v;
}

long evalAssignAdd(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalAdd, item1, item2);
}

long evalAssignSub(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalSub, item1, item2);
}

long evalAssignMult(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalMult, item1, item2);
}

long evalAssignDiv(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalDiv, item1, item2);
}

long evalAssignMod(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalMod, item1, item2);
}

long evalAssignLsh(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalLsh, item1, item2);
}

long evalAssignRsh(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalRsh, item1, item2);
}

long evalAssignAnd(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalBitand, item1, item2);
}

long evalAssignXor(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalBitxor, item1, item2);
}

long evalAssignOr(struct stackItemS *item1, struct stackItemS *item2)
{
    return doEvalAssignExt(evalBitor, item1, item2);
}

long doEvalIncDec(int p, int plus, struct stackItemS *item1)
{
    long v = longValue(item1);
    char buffer[16];
    if (p)
    {
        if (plus)
        {
            v++;
        }
        else
        {
            v--;
        }

        if (item1->type == ITEM_VAR_PTR)
        {
            sprintf(buffer, "%ld", v);
            symtabEntrySetval(item1->pointer, buffer);
        }
    }
    else
    {
        int diff = plus ? 1 : -1;
        if (item1->type == ITEM_VAR_PTR)
        {
            sprintf(buffer, "%ld", v + diff);
            symtabEntrySetval(item1->pointer, buffer);
        }
    }
    return v;
}

long evalPostinc(struct stackItemS *item1, struct stackItemS *unused __attribute__((unused)))
{
    return doEvalIncDec(0, 1, item1);
}

long evalPostdec(struct stackItemS *item1, struct stackItemS *unused __attribute__((unused)))
{
    return doEvalIncDec(0, 0, item1);
}

long evalPreinc(struct stackItems *item1, struct stackItemS *unused __attribute__((unused)))
{
    return doEvalIncDec(1, 1, item1);
}

long evalPredec(struct stackItemS *item1, struct stackItemS *unused __attribute__((unused)))
{
    return doEvalIncDec(1, 0, item1);
}

// Operator List
#define CH_GT 2            // greater than
#define CH_LT 3            // lesser than
#define CH_GE 4            // greater than or equals
#define CH_LE 5            // lesser than or equals
#define CH_RSH 6           // shift right
#define CH_LSH 7           // shitf left
#define CH_NE 8            // not equals
#define CH_EQ 9            // equals
#define CH_ASSIGN 10       // assignment
#define CH_PRE_INC 11      // pre-increment op
#define CH_POST_INC 12     // post-increment op
#define CH_PRE_DEC 13      // pre-decrement op
#define CH_POST_DEC 14     // post-decrement op
#define CH_B_AND 15        // bitwise AND
#define CH_B_OR 16         // bitwise OR
#define CH_B_XOR 17        // bitwise XOR
#define CH_AND 18          // logical AND
#define CH_OR 19           // logical OR
#define CH_EXP 20          // exponent or **
#define CH_MINUS 21        // unary minus
#define CH_PLUS 22         // unary plus
#define CH_ASSIGN_PLUS 23  // +=
#define CH_ASSIGN_MINUS 24 // -=
#define CH_ASSIGN_MULT 25  // *=
#define CH_ASSIGN_DIV 26   // /=
#define CH_ASSIGN_MOD 27   // %=
#define CH_ASSIGN_LSH 28   // <<=
#define CH_ASSIGN_RSH 29   // >>=
#define CH_ASSIGN_AND 30   // &=
#define CH_ASSIGN_XOR 31   // ^=
#define CH_ASSIGN_OR 32    // |=

enum
{
    ASSOC_NONE = 0,
    ASSOC_LEFT,
    ASSOC_RIGHT
};

struct opS
{
    char op;
    int prec;
    int assoc;
    char unary;
    char chars;
    long (*eval)(struct stackItemS *item1, struct stackItemS *item2);
} arithmOps[] =
    {
        {CH_POST_INC, 20, ASSOC_LEFT, 1, 2, evalPostinc},
        {CH_POST_DEC, 20, ASSOC_LEFT, 1, 2, evalPostdec},
        {CH_PRE_INC, 19, ASSOC_RIGHT, 1, 2, evalPostinc},
        {CH_PRE_DEC, 19, ASSOC_RIGHT, 1, 2, evalPostdec},
        {CH_MINUS, 19, ASSOC_RIGHT, 1, 1, evalUminus},
        {CH_PLUS, 19, ASSOC_RIGHT, 1, 1, evalUplus},
        {'!', 19, ASSOC_RIGHT, 1, 1, evalLognot},
        {'~', 19, ASSOC_RIGHT, 1, 1, evalBitnot},
        {CH_EXP, 18, ASSOC_RIGHT, 0, 2, evalExp},
        {'*', 17, ASSOC_LEFT, 0, 1, evalMult},
        {'/', 17, ASSOC_LEFT, 0, 1, evalDiv},
        {'%', 17, ASSOC_LEFT, 0, 1, evalMod},
        {'+', 16, ASSOC_LEFT, 0, 1, evalAdd},
        {'-', 16, ASSOC_LEFT, 0, 1, evalSub},
        {CH_LSH, 15, ASSOC_LEFT, 0, 2, evalLsh},
        {CH_RSH, 15, ASSOC_LEFT, 0, 2, evalRsh},
        {'<', 14, ASSOC_LEFT, 0, 1, evalLt},
        {CH_LE, 14, ASSOC_LEFT, 0, 2, evalLe},
        {'>', 14, ASSOC_LEFT, 0, 1, evalGt},
        {CH_GE, 14, ASSOC_LEFT, 0, 2, evalGe},
        {CH_EQ, 13, ASSOC_LEFT, 0, 2, evalEq},
        {CH_NE, 13, ASSOC_LEFT, 0, 2, evalNe},
        {'&', 12, ASSOC_LEFT, 0, 1, evalBitand},
        {'^', 11, ASSOC_LEFT, 0, 1, evalBitxor},
        {'|', 10, ASSOC_LEFT, 0, 1, evalBitor},
        {CH_AND, 9, ASSOC_LEFT, 0, 2, evalLogand},
        {CH_OR, 8, ASSOC_LEFT, 0, 2, evalLogor},
        {CH_ASSIGN, 7, ASSOC_RIGHT, 0, 1, evalAssign},
        {CH_ASSIGN_PLUS, 7, ASSOC_RIGHT, 0, 2, evalAssignAdd},
        {CH_ASSIGN_MINUS, 7, ASSOC_RIGHT, 0, 2, evalAssignSub},
        {CH_ASSIGN_MULT, 7, ASSOC_RIGHT, 0, 2, evalAssignMult},
        {CH_ASSIGN_DIV, 7, ASSOC_RIGHT, 0, 2, evalAssignDiv},
        {CH_ASSIGN_MOD, 7, ASSOC_RIGHT, 0, 2, evalAssignMod},
        {CH_ASSIGN_LSH, 7, ASSOC_RIGHT, 0, 3, evalAssignLsh},
        {CH_ASSIGN_RSH, 7, ASSOC_RIGHT, 0, 3, evalAssignRsh},
        {CH_ASSIGN_AND, 7, ASSOC_RIGHT, 0, 2, evalAssignAnd},
        {CH_ASSIGN_XOR, 7, ASSOC_RIGHT, 0, 2, evalAssignXor},
        {CH_ASSIGN_OR, 7, ASSOC_RIGHT, 0, 2, evalAssignOr},

        {'(', 0, ASSOC_NONE, 0, 1, NULL},
        {')', 0, ASSOC_NONE, 0, 1, NULL}};

struct opS *OP_POST_INC = &arithmOps[0];
struct opS *OP_POST_DEC = &arithmOps[1];
struct opS *OP_PRE_INC = &arithmOps[2];
struct opS *OP_PRE_DEC = &arithmOps[3];
struct opS *OP_UMINUS = &arithmOps[4];
struct opS *OP_UPLUS = &arithmOps[5];
struct opS *OP_LOG_NOT = &arithmOps[6];
struct opS *OP_BIT_NOT = &arithmOps[7];
struct opS *OP_EXP = &arithmOps[8];
struct opS *OP_MULT = &arithmOps[9];
struct opS *OP_DIV = &arithmOps[10];
struct opS *OP_MOD = &arithmOps[11];
struct opS *OP_ADD = &arithmOps[12];
struct opS *OP_SUB = &arithmOps[13];
struct opS *OP_LSH = &arithmOps[14];
struct opS *OP_RSH = &arithmOps[15];
struct opS *OP_LT = &arithmOps[16];
struct opS *OP_LE = &arithmOps[17];
struct opS *OP_GT = &arithmOps[18];
struct opS *OP_GE = &arithmOps[19];
struct opS *OP_EQ = &arithmOps[20];
struct opS *OP_NE = &arithmOps[21];
struct opS *OP_BIT_AND = &arithmOps[22];
struct opS *OP_BIT_XOR = &arithmOps[23];
struct opS *OP_BIT_OR = &arithmOps[24];
struct opS *OP_LOG_AND = &arithmOps[25];
struct opS *OP_LOG_OR = &arithmOps[26];
struct opS *OP_ASSIGN = &arithmOps[27];
struct opS *OP_ASSIGN_ADD = &arithmOps[28];
struct opS *OP_ASSIGN_SUB = &arithmOps[29];
struct opS *OP_ASSIGN_MULT = &arithmOps[30];
struct opS *OP_ASSIGN_DIV = &arithmOps[31];
struct opS *OP_ASSIGN_MOD = &arithmOps[32];
struct opS *OP_ASSIGN_LSH = &arithmOps[33];
struct opS *OP_ASSIGN_RSH = &arithmOps[34];
struct opS *OP_ASSIGN_AND = &arithmOps[35];
struct opS *OP_ASSIGN_XOR = &arithmOps[36];
struct opS *OP_ASSIGN_OR = &arithmOps[37];
struct opS *OP_LBRACE = &arithmOps[38];
struct opS *OP_RBRACE = &arithmOps[39];

// Returns 1 if param is valid
int validNameChar(char c)
{
    switch (c)
    {
    case '_':
    case '@':
    case '#':
    case '$':
    case '?':
        return 1;

    default:
        if (isalnum(c))
        {
            return 1;
        }
        return 0;
    }
}

// Get operator from expression
struct opS *getOp(char *ex)
{
    switch (*ex)
    {
    case '+':
        if (ex[1] == '+')
        {
            return OP_POST_INC;
        }
        else if (ex[1] == '=')
        {
            return OP_ASSIGN_ADD;
        }
        return OP_ADD;

    case '-':
        if (ex[1] == '-')
        {
            return OP_POST_DEC;
        }
        else if (ex[1] == '=')
        {
            return OP_ASSIGN_SUB;
        }
        return OP_SUB;

    case '*':
        if (ex[1] == '*')
        {
            return OP_EXP;
        }
        else if (ex[1] == '=')
        {
            return OP_ASSIGN_MULT;
        }
        return OP_MULT;

    case '<':
        if (ex[1] == '<')
        {
            if (ex[2] == '=')
            {
                return OP_ASSIGN_LSH;
            }
            return OP_LSH;
        }
        else if (ex[1] == '=')
        {
            return OP_LE;
        }
        return OP_LT;

    case '>':
        if (ex[1] == '>')
        {
            if (ex[2] == '=')
            {
                return OP_ASSIGN_RSH;
            }
            return OP_RSH;
        }
        else if (ex[1] == '=')
        {
            return OP_GE;
        }
        return OP_GT;

    case '!':
        if (ex[1] == '=')
        {
            return OP_NE;
        }
        return OP_LOG_NOT;

    case '=':
        if (ex[1] == '=')
        {
            return OP_EQ;
        }
        return OP_ASSIGN;

    case '&':
        if (ex[1] == '&')
        {
            return OP_LOG_AND;
        }
        else if (ex[1] == '=')
        {
            return OP_ASSIGN_AND;
        }
        return OP_BIT_AND;

    case '|':
        if (ex[1] == '|')
        {
            return OP_LOG_OR;
        }
        else if (ex[1] == '=')
        {
            return OP_ASSIGN_OR;
        }
        return OP_BIT_OR;

    case '^':
        if (ex[1] == '=')
        {
            return OP_ASSIGN_XOR;
        }
        return OP_BIT_XOR;
    case '/':
        if (ex[1] == '=')
        {
            return OP_ASSIGN_DIV;
        }
        return OP_DIV;

    case '%':
        if (ex[1] == '=')
        {
            return OP_ASSIGN_MOD;
        }
        return OP_MOD;

    case '~':
        return OP_BIT_NOT;

    case '(':
        return OP_LBRACE;

    case ')':
        return OP_RBRACE;
    }
    return NULL;
}

// Push operator to stack
void pushOpstack(struct opS *op)
{
    if (nopStack > MAXOPSTACK - 1)
    {
        fprintf(stderr, "Error: Operator stack overflow\n");
        error = 1;
        return;
    }
    opstack[nopStack++] = op;
}

// Pop operator to stack
struct opS *popOpstack(void)
{
    if (!nopStack)
    {
        fprintf(stderr, "Error: Operator stack empty\n");
        error = 1;
        return NULL;
    }
    return opstack[--nopStack];
}

// Push numeric op to stack
void pushNumstackl(long val)
{
    if (nNumStack > MAXNUMSTACK - 1)
    {
        fprintf(stderr, "Error: Number stack overflow\n");
        error = 1;
        return;
    }

    numstack[nNumStack].type = ITEM_LONG_INT;
    numstack[nNumStack++].val = val;
}

// Push var op to op stack
void pushNumstackv(struct symtabEntryS *val)
{

    if (nNumStack > MAXNUMSTACK - 1)
    {
        fprintf(stderr, "Error: Number stack overflow\n");
        error = 1;
        return;
    }

    numStack[nNumStack].type = ITEM_VAR_PTR;
    numStack[nNumStack++].pointer = val;
}

// Pop op from op stack
struct stackItemS popNumstack(void)
{
    if (!nNumStack)
    {
        fprintf(stderr, "Error: Number stack empty\n");
        error = 1;
        return (struct stackItemS){};
    }
    return numStack[--nNumStack];
}

// Operator shunting
void shuntOp(struct ops *op)
{
    struct opS *pop;
    error = 0;
    if (op->op == '(')
    {
        pushOpstack(op);
        return;
    }
    else if (op->op == ')')
    {
        while (nopStack > 0 && opstack[nopStack - 1]->op != '(')
        {
            pop = popOpstack();
            if (error)
            {
                return;
            }
            struct stackItemS num1 = popNumstack();
            if (error)
            {
                return;
            }
            if (pop->unary)
            {
                pushNumstackl(pop->eval(&num1, 0));
            }
            else
            {
                struct stackItemS num2 = popNumstack();
                if (error)
                {
                    return;
                }
                pushNumstackl(pop->eval(&num2, &num1));
                if (error)
                {
                    return;
                }
            }
        }
        if (!(pop = popOpstack()) || pop->op != '(')
        {
            fprintf(stderr, "Error: Stack error. No matching \'(\'\n");
            error = 1;
        }
        return;
    }

    if (op->assoc == ASSOC_RIGHT)
    {
        while (nopStack && op->prec < opstack[nopStack - 1]->prec)
        {
            pop = popOpstack();
            if (error)
            {
                return;
            }
            struct stackItemS num1 = popNumstack();
            if (pop->unary)
            {
                pushNumstackl(pop->eval(&num1, 0));
            }
            else
            {
                struct stackItemS num2 = popNumstack();
                if (error)
                {
                    return;
                }
                pushNumstackl(pop->eval(&num2, &num1));
            }
            if (error)
            {
                return;
            }
        }
    }
    else
    {
        while (nopStack && op->prec <= opstack[nopStack - 1]->prec)
        {
            pop = popOpstack();
            if (error)
            {
                return;
            }
            struct stackItemS num1 = popNumstack();
            if (pop->unary)
            {
                pushNumstackl(pop->eval(&num1, 0));
            }
            else
            {
                struct stackItemS num2 = popNumstack();
                if (error)
                {
                    return;
                }
                pushNumstackl(pop->eval(&num2, &num1));
            }
            if (error)
            {
                return;
            }
        }
    }
    push_opstack(op);
}

int getNdigit(char a, int b, int *res)
{

    if (!isalnum(a) && a != '@' && a != '_')
    {
        return 0;
    }

    char m, m2;

    if (b <= 10)
    {
        m = '0' + b - 1;
        if (a >= '0' && a <= m)
        {
            (*res) = a - '0';
            return 1;
        }
        goto invalid;
    }

    if (a >= '0' && a <= '9')
    {
        (*res) = a - '0';
        return 1;
    }

    if (b <= 36)
    {
        m = 'a' + b - 11;
        m2 = m - ('a' - 'A');
        if (a >= 'a' && a <= m)
        {
            (*res) = a - 'a' + 10;
            return 1;
        }
        if (a >= 'A' && a <= m2)
        {
            (*res) = a - 'A' + 10;
            return 1;
        }
    }

    else if (b <= 62)
    {

        if (a >= 'a' && a <= 'z')
        {
            (*res) = a - 'a' + 10;
            return 1;
        }
        m2 = 'A' + b - 37;
        if (a >= 'A' && a <= m2)
        {
            (*res) = a - 'A' + 36;
            return 1;
        }
    }
    else if (a == '@')
    {
        (*res) = 62;
        return 1;
    }
    else if (a == '_' && b == 64)
    {
        (*res) = 63;
        return 1;
    }

invalid:
    fprintf(stderr, "Error: digit %c exceeds the value of the base %d\n", a, b);
    error = 1;
    return 0;
}

long getNum(char *a, int *charCount)
{
    char *a2 = a;
    long n = 0;
    int n2, b = 10;

    if (*a2 == '0')
    {
        switch (a2[1])
        {
        case 'x':
        case 'X':
            b = 16;
            a2 += 2;
            break;

        case 'b':
        case 'B':
            b = 2;
            a2 += 2;
            break;

        default:
            b = 8;
            a2++;
            break;
        }
    }

    while (getNdigit(*a2, b, &n2))
    {
        n = (n * b) + n2;
        a2++;
    }

    if (error)
    {
        return 0;
    }

    if (b != 10)
    {
        (*charCount) = a2 - a;
        return n;
    }

    if (*a2 == '#')
    {
        b = n;
        n = 0;
        a2++;
        while (getNdigit(*a2, b, &n2))
        {
            n = (n * b) + n2;
            a2++;
        }
        if (error)
        {
            return 0;
        }
    }
    (*charCount) = a2 - a;
    return n;
}

struct symtabEntryS *getVar(char *a, int *charCount)
{
    char *aa = a;
    if (*aa == '$')
    {
        aa++;
    }
    char *a2 = aa;
    while (*a2 && validNameChar(*a2))
    {
        a2++;
    }
    int length = a2 - aa;

    if (length == 0)
    {
        (*charCount) = a2 - a;
        return NULL;
    }

    char name[length + 1];
    strncpy(name, aa, length);
    name[length] = '\0';

    struct symtabEntryS *e = getSymtabEntry(name);
    if (!e)
    {
        e = addToSymtab(name);
    }

    (*charCount) = a2 - a;
    return e;
}

char *arithmExpand(char *origExp)
{
    char *exp;
    char *start = NULL;
    struct opS startOp = {'X', 0, ASSOC_NONE, 0, 0, NULL};
    struct opS *op = NULL;
    int num1, num2;
    struct opS *lastOp = &startOp;

    int baseExpLength = strlen(origExp);
    char *baseExp = malloc(baseExpLength + 1);
    if (!baseExp)
    {
        fprintf(stderr, "Error: Not enough memory for arithmetic expansion\n");
        return NULL;
    }
    if (origExp[0] == '$' && origExp[1] == '(' && origExp[2] == '(')
    {
        strcpy(baseExp, origExp + 3);
        baseExpLength -= 3;

        if (baseExp[baseExpLength - 1] == ')' && baseExp[baseExpLength - 2] == ')')
        {
            baseExp[baseExpLength - 2] = '\0';
        }
    }
    else
    {
        strcpy(baseExp, origExp);
    }

    nopStack = 0;
    nNumStack = 0;
    error = 0;
    exp = baseExp;

    for (; *exp;)
    {
        if (!start)
        {
            if ((op = getOp(exp)))
            {
                if (lastOp && (lastOp == &startOp || lastOp->op != ')'))
                {
                    if (op->op == '-')
                    {
                        op = OP_UMINUS;
                    }
                    else if (op->op == '+')
                    {
                        op = OP_UPLUS;
                    }
                    else if (op->op != '(' && !op->unary)
                    {
                        fprintf(stderr, "Error: Illegal use of binary operator (%c)\n", op->op);
                        goto err;
                    }
                }
                if (op->op == CH_POST_INC || op->op == CH_POST_DEC)
                {
                    if (exp < baseExp + 2 || !validNameChar(exp[-2]))
                    {
                        if (op == OP_POST_INC)
                        {
                            op = OP_PRE_INC;
                        }
                        else
                        {
                            op = OP_PRE_DEC;
                        }
                    }
                }
                error = 0;
                shuntOp(op);
                if (error)
                {
                    goto err;
                }
                lastOp = op;
                exp += op->chars;
            }
            else if (validNameChar(*exp))
            {
                start = exp;
            }
            else if (isspace(*exp))
            {
                exp++;
            }
            else
            {
                fprintf(stderr, "error: Syntax error near: %s\n", exp);
                goto err;
            }
        }
        else
        {
            if (isspace(*exp))
            {
                exp++;
            }
            else if (isdigit(*exp))
            {
                error = 0;
                num1 = getNum(start, &num2);
                if (error)
                {
                    goto err;
                }
                pushNumstackl(num1);
                if (error)
                {
                    goto err;
                }
                start = NULL;
                lastOp = NULL;
                exp += num2;
            }
            else if (validNameChar(*exp))
            {
                struct symtabEntryS *num1 = getVar(start, &num2);
                if (!num1)
                {
                    fprintf(stderr, "Error: Couldn not add symbol near: %s\n", start);
                    goto err;
                }
                error = 0;
                pushNumstackv(num1);
                if (error)
                {
                    goto err;
                }
                start = NULL;
                lastOp = NULL;
                exp += num2;
            }
            else if ((op = getOp(exp)))
            {
                error = 0;
                num1 = getNum(start, &num2);
                if (error)
                {
                    goto err;
                }
                pushNumstackl(num1);
                if (error)
                {
                    goto err;
                }
                start = NULL;

                if (op->op == CH_POST_INC || op->op == CH_POST_DEC)
                {
                    if (exp < baseExp + 2 || !validNameChar(exp[-2]))
                    {
                        if (op == OP_POST_INC)
                        {
                            op = OP_PRE_INC;
                        }
                        else
                        {
                            op = OP_PRE_DEC;
                        }
                    }
                }

                shuntOp(op);
                if (error)
                {
                    goto err;
                }
                lastOp = op;
                exp += op->chars;
            }
            else
            {
                fprintf(stderr, "error: Syntax error near: %s\n", exp);
                goto err;
            }
        }
    }

    if (start)
    {
        error = 0;
        if (isdigit(*start))
        {
            num1 = getNum(start, &num2);
            if (error)
            {
                goto err;
            }
            pushNumstackl(num1);
        }
        else if (validNameChar(*start))
        {
            pushNumstackv(getVar(start, &num2));
        }
        if (error)
        {
            goto err;
        }
    }

    while (nopStack)
    {
        error = 0;
        op = popOpstack();
        if (error)
        {
            goto err;
        }
        struct stackItemS num1 = popNumstack();
        if (error)
        {
            goto err;
        }
        if (op->unary)
        {
            pushNumstackl(op->eval(&num1, 0));
        }
        else
        {
            struct stackItemS num2 = popNumstack();
            if (error)
            {
                goto err;
            }
            pushNumstackl(op->eval(&num2, &num1));
        }
        if (error)
        {
            goto err;
        }
    }

    if (!nNumStack)
    {
        free(baseExp);
        return NULL;
    }

    if (nNumStack != 1)
    {
        fprintf(stderr, "Error: Number stack has %d elements after evaluation. Should be 1.\n", nNumStack);
        goto err;
    }

    char result[64];
    sprintf(result, "%ld", numstack[0].val);
    char *result2 = malloc(strlen(result) + 1);
    if (result2)
    {
        strcpy(result2, result);
    }
    free(baseExp);
    return result2;

err:
    free(baseExp);
    return NULL;
}