#define _DEFAULT_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "simulate.h"
#include "main.h"
#include "ops.h"
#include "strings.h"

#define STACK_CAPACITY 4096

char **simulated_argv = NULL;
int simulated_argc;

typedef int64_t word;

word stack[STACK_CAPACITY] = {0};
int sp = 0;

static Operation *current_op;

static void simulate_call(Operation *ops);
static Operation *simulate_op(Operation *op);
static void push(word c);
static word pop(void);

static void push(word c)
{
    if (sp == STACK_CAPACITY) {
	tokerror(current_op->tok, "simulation stack overflow\n");
	fatal("simulate: stack overflow!");
    }
    stack[sp++] = c;
}

static word pop(void)
{
    if (sp == 0) {
	tokerror(current_op->tok, "simulation stack underflow\n");
	fatal("simulate: stack underflow!");
    }
    return stack[--sp];
}

static void simulate_call(Operation *ops)
{
    Operation *op;

    for (op = ops; op != NULL && op->op != OP_RETURN; op = simulate_op(op));
}

static Operation *simulate_op(Operation *op)
{
    word a, b, c, d, e, f, g;
    Operation *next_op, *tmp_op;

    static_assert(OP_COUNT == 74, "simulate_op: exhausetive op handling");
    next_op = op->next;
    current_op = op;
    switch (op->op) {
    case OP_PUSH:
	push(op->operand.intr);
	break;
    case OP_PUSH_ADDR:
	push((word)op->operand.variable->addr);
	break;
    case OP_PUSH_STR:
	push(op->operand.str.len - escape_chars(op));
	push((word)unescape_chars(op));
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
    case OP_DIVMOD:
	a = pop();
	b = pop();
	push(b % a);
	push(b / a);
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
	fflush(stdout);
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
    case OP_2DROP:
	pop();
	pop();
	break;
    case OP_DUP:
	a = pop();
	push(a);
	push(a);
	break;
    case OP_2DUP:
	a = pop();
	b = pop();
	push(b);
	push(a);
	push(b);
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
    case OP_2OVER:
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	push(d);
	push(c);
	push(b);
	push(a);
	push(d);
	push(c);
	break;
    case OP_SWAP:
	a = pop();
	b = pop();
	push(a);
	push(b);
	break;
    case OP_2SWAP:
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	push(b);
	push(a);
	push(d);
	push(c);
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
    case OP_ONEPLUS:
	push(pop() + 1);
	break;
    case OP_ONEMINUS:
	push(pop() - 1);
	break;
    case OP_EQUAL:
	push(-(pop() == pop()));
	break;
    case OP_0EQUAL:
	push(-(pop() == 0));
	break;
    case OP_NEQUAL:
	push(pop() != pop());
	break;
    case OP_GT:
	a = pop();
	b = pop();
	push(-(b > a));
	break;
    case OP_0GT:
	push(-(pop() > 0));
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
    case OP_0LT:
	push(-(pop() < 0));
	break;
    case OP_LE:
	a = pop();
	b = pop();
	push(-(b <= a));
	break;
    case OP_IF:
	a = pop();
	if (a == 0 && op->operand.if_op.else_op != NULL) {
	    op->operand.if_op.if_true = false;
	    return op->operand.if_op.else_op->next;
	} else if (a == 0) {
	    op->operand.if_op.if_true = false;
	    return op->operand.if_op.endif_op->next;
	} else {
	    op->operand.if_op.if_true = true;
	    return op->next;
	}
	break;
    case OP_ELSE:
	tmp_op = op->operand.if_op.ifop_op;
	if (tmp_op->operand.if_op.if_true == true) {
	    return tmp_op->operand.if_op.endif_op;
	} else {
	    return op->next;
	}
	break;
    case OP_ENDIF:
	return op->next;
	//	next_op = simulate_conditional(op);
	break;
    case OP_ENDIFS:
	return op->next;
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
	tmp_op->operand.doloop_op.current += a;
	if (tmp_op->operand.doloop_op.end < tmp_op->operand.doloop_op.start
	    && tmp_op->operand.doloop_op.current <= tmp_op->operand.doloop_op.end) {
	    return op->next;
	} else if (tmp_op->operand.doloop_op.end > tmp_op->operand.doloop_op.start
		   && tmp_op->operand.doloop_op.current >= tmp_op->operand.doloop_op.end) {
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
    case OP_BEGIN:
	break;
    case OP_WHILE:
	a = pop();
	if (a == 0) {
	    assert(op->operand.indef_op.repeat_op != NULL);
	    return op->operand.indef_op.repeat_op->next;
	} else {
	    return op->next;
	}
	break;
    case OP_REPEAT:
	assert(op->operand.indef_op.begin_op != NULL);
	return op->operand.indef_op.begin_op;
    case OP_UNTIL:
	assert(op->operand.indef_op.begin_op != NULL);
	assert(op->operand.indef_op.while_op == NULL);
	a = pop();
	if (a == 0) {
	    return op->next;
	} else {
	    return op->operand.indef_op.begin_op;
	}
    case OP_LEAVE:
	assert(op->operand.indef_op.leave_jump != NULL);
	assert(*op->operand.indef_op.leave_jump != NULL);
	return (*op->operand.indef_op.leave_jump)->next;
	break;
    case OP_SYS0:
	a = pop();
	push(syscall(a));
	break;
    case OP_SYS1:
	a = pop();
	b = pop();
	/*
	 * We need to make special allowance for the exit
	 * system call during simulation because the C library
	 * needs to do some cleanup.
	 */
	if (a == SYS_exit) {
	    exit(b);
	}
	push(syscall(a, b));
	break;
    case OP_SYS2:
	a = pop();
	b = pop();
	c = pop();
	push(syscall(a, b, c));
	break;
    case OP_SYS3:
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	push(syscall(a, b, c, d));
	break;
    case OP_SYS4:
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	e = pop();
	push(syscall(a, b, c, d, e));
	break;
    case OP_SYS5:
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	e = pop();
	f = pop();
	push(syscall(a, b, c, d, e, f));
	break;
    case OP_SYS6:
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	e = pop();
	f = pop();
	g = pop();
	push(syscall(a, b, c, d, e, f, g));
	break;
    case OP_STORE8:
	a = pop();
	b = pop();
	*(uint8_t *)a = (uint8_t)b;
	break;
    case OP_STORE16:
	a = pop();
	b = pop();
	*(uint16_t *)a = (uint16_t)b;
	break;
    case OP_STORE32:
	a = pop();
	b = pop();
	*(uint32_t *)a = (uint32_t)b;
	break;
    case OP_STORE64:
	a = pop();
	b = pop();
	*(uint64_t *)a = (uint64_t)b;
	break;
    case OP_LOAD8:
	a = pop();
	push((word)(*(uint8_t *)a));
	break;
    case OP_LOAD16:
	a = pop();
	push((word)(*(uint16_t *)a));
	break;
    case OP_LOAD32:
	a = pop();
	push((word)(*(uint32_t *)a));
	break;
    case OP_LOAD64:
	a = pop();
	push((word)(*(uint64_t *)a));
	break;
    case OP_ARGC:
	push(simulated_argc);
	break;
    case OP_ARGV:
	push((word)simulated_argv);
	break;
    case OP_CALL:
	op->operand.call_op.ret_addr = op->next;
	simulate_call(op->operand.call_op.function->ops);
	break;
    case OP_RETURN:
	tokerror(op->tok, "'return' operation outside function\n");
	break;
    case OP_UNKNOWN:
    case OP_COUNT:
	unreachable("simulate_op");
    }
    return next_op;
}

void simulate(void)
{
    Operation *op;

    for (op = operations; op != NULL; op = simulate_op(op));
}
