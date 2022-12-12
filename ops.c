#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"
#include "ops.h"
#include "main.h"

Operation *operations = NULL;

struct {
    char *opname;
    OpWord op;
} op_table[] = {
    { "", OP_PUSH},
    { "+", OP_ADD},
    { "-", OP_MINUS},
    { "*", OP_MULTIPLY},
    { "/", OP_DIVIDE},
    { "mod", OP_MOD},
    { "and", OP_BAND},
    { "or", OP_BOR},
    { "xor", OP_BXOR},
    { "negate", OP_NEGATE},
    { ".", OP_PRINT},
    { "abs", OP_ABS},
    { "min", OP_MIN},
    { "max", OP_MAX},
    
    { "", OP_UNKNOWN},
};

static OpWord find_op_in_table(char *opname);
static Operation *newoperation(void);
static Operation *token_to_op(Token *tok);

static OpWord find_op_in_table(char *opname)
{
    for (int i = 0; i < OP_COUNT; i++) {
        if (strcmp(opname, op_table[i].opname) == 0) {
	    return op_table[i].op;
	}
    }
    return OP_UNKNOWN;
}

static Operation *newoperation(void)
{
    Operation *new_op;

    new_op = malloc(sizeof(*new_op));
    if (new_op == NULL) {
	fatal("newoperation: malloc returned NULL!");
    }
    return new_op;
}

static Operation *token_to_op(Token *tok)
{
    Operation *new_op;
    OpWord op;
    char *p;
    
    new_op = newoperation();
    new_op->next = NULL;
    if ((op = find_op_in_table(tok->text)) != OP_UNKNOWN) {
	new_op->op = op;
	return new_op;
    }
    p = tok->text;
    if (*p == '+' || *p == '-' || isdigit(*p)) {
	if (*p == '+' || *p == '-') {
	    p++;
	}
	while (p - tok->text < tok->length) {
	    if (*p == '.') {
		not_implemented("Floating point constants");
	    }
	    if (!isdigit(*p)) {
		tokerror(tok, "Invalid numerical constant '%s'\n", tok->text);
	    }
	    p++;
	}
	new_op->operand.intr = atoi(tok->text);
	new_op->op = OP_PUSH;
	return new_op;
    }
    tokerror(tok, "Unrecognised wprd '%s'\n", tok->text);
    return NULL;
}

void tokens_to_ops(void)
{
    Token *tokptr;
    Operation *opptr, *new_op;

    opptr = NULL;
    for (tokptr = tokens; tokptr != NULL; tokptr = tokptr->next) {
	new_op = token_to_op(tokptr);
	if (operations == NULL) {
	    operations = new_op;
	    opptr = operations;
	} else {
	    opptr->next = new_op;
	    opptr = opptr->next;
	}
    }
    /* for (opptr = operations; opptr != NULL; opptr = opptr->next) {
	printf("Op %d\n", opptr->op);
	} */
}
