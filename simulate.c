#include <stdlib.h>
#include <stdint.h>
#include "simulate.h"
#include "main.h"
#include "ops.h"

#define STACK_CAPACITY 4096

typedef int64_t word;

word stack[STACK_CAPACITY] = {0};
int sp = 0;

static void push(word c);
static word pop(void);

static void push(word c)
{
    if (sp == STACK_CAPACITY) {
	fatal("simulate: stack overflow!");
    }
    stack[sp++] = c;
}

static word pop(void)
{
    if (sp == 0) {
	fatal("simulate: stack underflow!");
    }
    return stack[--sp];
}

static void simulate_op(Operation *op)
{
    word a, b, c;
    switch (op->op) {
    case OP_PUSH:
	push(op->operand.intr);
	break;
    case OP_ADD:
	push(pop() + pop());
	break;
    case OP_MINUS:
	a = pop();
	b = pop();
	push(b - a);
	break;
    case OP_MULTIPLY:
	push(pop() * pop());
	break;
    case OP_DIVIDE:
	a = pop();
	b = pop();
	push(b / a);
	break;
    case OP_MOD:
	a = pop();
	b = pop();
	push(b % a);
	break;
    case OP_BAND:
	push(pop() & pop());
	break;
    case OP_BOR:
	push(pop() | pop());
	break;
    case OP_BXOR:
	push(pop() ^ pop());
	break;
    case OP_NEGATE:
	push(-pop());
	break;
    case OP_PRINT:
	printf("%ld\n", pop());
	break;
    case OP_ABS:
	push(labs(pop()));
	break;
    case OP_MAX:
	a = pop();
	b = pop();
	push(a > b ? a : b);
	break;
    case OP_MIN:
	a = pop();
	b = pop();
	push(a < b ? a : b);
	break;
    case OP_DROP:
	pop();
	break;
    case OP_DUP:
	a = pop();
	push(a);
	push(a);
	break;
    case OP_ROT:
	a = pop();
	b = pop();
	c = pop();
	push(b);
	push(a);
	push(c);
	break;
    case OP_OVER:
	a = pop();
	b = pop();
	push(b);
	push(a);
	push(b);
	break;
    case OP_SWAP:
	a = pop();
	b = pop();
	push(a);
	push(b);
	break;
    case OP_UNKNOWN:
    case OP_COUNT:
	unreachable("simulate_op");
    }
}

void simulate(void)
{
    Operation *op;

    for (op = operations; op != NULL; op = op->next) {
	simulate_op(op);
    }
}
