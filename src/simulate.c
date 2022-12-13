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
    Operation *next_op, *tmp_op;

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
    case OP_DO:
	a = pop();
	b = pop();
	op->operand.doloop_op.start = a;
	op->operand.doloop_op.end = b;
	op->operand.doloop_op.current = a;
	return op->next;
    case OP_LOOP_PLUS:
	a = pop();
	tmp_op = op->operand.doloop_op.do_op;
	if ((tmp_op->operand.doloop_op.current += a) >= tmp_op->operand.doloop_op.end) {
	    return op->next;
	} else {
	    return tmp_op->next;
	}
	break;
    case OP_LOOP:
	tmp_op = op->operand.doloop_op.do_op;
	if (++tmp_op->operand.doloop_op.current == tmp_op->operand.doloop_op.end) {
	    return op->next;
	} else {
	    return tmp_op->next;
	}
	break;
    case OP_I:
	tmp_op = op->operand.doloop_op.do_op;
	push(tmp_op->operand.doloop_op.current);
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
    Operation *opptr;
    
    switch (op->op) {
    case OP_IF:
	a = pop();
	if (op->operand.if_op.endif_op == NULL) {
	    tokerror(op->tok, "'if' operand not terminated with 'endif'");
	}
	if (a == 0) {
	    /* Go to 'else' or endif */
	    Operation *jump_to;

	    jump_to = op->operand.if_op.else_op != NULL ? op->operand.if_op.else_op : op->operand.if_op.endif_op;
	    opptr = op;
	    while (opptr != NULL && opptr != jump_to)
		opptr = opptr->next;
	    if (opptr == NULL) {
		tokerror(op->tok, "'if' operand not terminated with 'endif'");
	    }
	    return opptr->next;
	} else {
	    /* Do stuff in the if block and then go to endif */
	    Operation *jump_to;
	    opptr = op;
	    jump_to = op->operand.if_op.endif_op;
	    while (opptr != NULL && opptr != jump_to && opptr->op != OP_ELSE)
		opptr = opptr->next;
	    if (opptr == NULL) {
		tokerror(op->tok, "'if' operand not terminated with 'endif'");
	    }
	    opptr->next = jump_to->next;
	    return op->next;
	}
	break;
    case OP_ELSE:
	return op->next;
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