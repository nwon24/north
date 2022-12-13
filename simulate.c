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

static Operation *simulate_conditional(Operation *op);

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

static Operation *simulate_op(Operation *op)
{
    word a, b, c;
    Operation *next_op;

    next_op = op->next;
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
    case OP_LSHIFT:
	a = pop();
	b = pop();
	push(b << a);
	break;
    case OP_RSHIFT:
	a = pop();
	b = pop();
	push(b >> a);
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
    case OP_NIP:
	a = pop();
	b = pop();
	push(a);
	break;
    case OP_TUCK:
	a = pop();
	b = pop();
	push(a);
	push(b);
	push(a);
	break;
    case OP_EQUAL:
	push(pop() == pop());
	break;
    case OP_NEQUAL:
	push(pop() != pop());
	break;
    case OP_GT:
	a = pop();
	b = pop();
	push(-(b > a));
	break;
    case OP_GE:
	a = pop();
	b = pop();
	push(-(b >= a));
	break;
    case OP_LT:
	a = pop();
	b = pop();
	push(-(b < a));
	break;
    case OP_LE:
	a = pop();
	b = pop();
	push(-(b <= a));
	break;
    case OP_IF:
    case OP_ELSE:
    case OP_ENDIF:
	next_op = simulate_conditional(op);
	break;
    case OP_UNKNOWN:
    case OP_COUNT:
	unreachable("simulate_op");
    }
    return next_op;
}

static Operation *simulate_conditional(Operation *op)
{
    word a;
    Operation *else_or_endif;
    
    switch (op->op) {
    case OP_IF:
	a = pop();
	if (op->operand.else_or_endif == NULL) {
	    tokerror(op->tok, "'if' operand not terminated with 'else' or 'endif'");
	}
	else_or_endif = op->operand.else_or_endif;
	if (a == 0) {
	    else_or_endif->operand.else_or_endif = else_or_endif;
	    else_or_endif->operand.else_or_endif = else_or_endif->next;
	    return else_or_endif;
	} else {
	    return op->next;
	}
	break;
    case OP_ELSE:
	if (op->operand.else_or_endif == NULL) {
	    tokerror(op->tok, "'else' operand not terminated with 'else' or 'endif'");
	}
	return op->operand.else_or_endif;
    case OP_ENDIF:
	return op->next;
    default:
	unreachable("simulate_conditional");
    }
    unreachable("simulate_conditional");
    return NULL;
}


void simulate(void)
{
    Operation *op;

    for (op = operations; op != NULL; op = simulate_op(op));
}
