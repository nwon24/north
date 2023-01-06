#include <assert.h>

#include "main.h"
#include "branches.h"
#include "ops.h"

void cross_reference_branches(Operation *ops)
{
    int block_addr = 0, i;
    Operation *opptr;
    Operation *conditional_ops[MAX_NESTED_BRANCHES];
    Operation *do_ops[MAX_NESTED_BRANCHES];
    Operation *while_ops[MAX_NESTED_BRANCHES];
    Operation *begin_ops[MAX_NESTED_BRANCHES];
    int cond_op_ptr;
    int do_op_ptr;
    int while_op_ptr;
    int begin_op_ptr;

    opptr = ops;
    cond_op_ptr = 0;
    do_op_ptr = 0;
    while_op_ptr = 0;
    begin_op_ptr = 0;
    block_addr = 0;
    static_assert(OP_COUNT == 77, "cross_reference_branches: exhausetive op handling");
    while (opptr != NULL) {
	switch (opptr->op) {
	case  OP_IF:
	    if (cond_op_ptr >= MAX_NESTED_BRANCHES) {
		tokerror(opptr->tok, "Too many nested branches\n");
	    }
	    opptr->operand.if_op.else_op = NULL;
	    opptr->operand.if_op.endif_op = NULL;
	    opptr->operand.if_op.ifop_op = NULL;
	    conditional_ops[cond_op_ptr++] = opptr;
	    break;
	case  OP_ELSE:
	    if (cond_op_ptr == 0) {
		tokerror(opptr->tok, "'else' operation with no preceding 'if'\n");
	    }
	    opptr->operand.if_op.else_op = NULL;
	    opptr->operand.if_op.endif_op = NULL;
	    conditional_ops[cond_op_ptr-1]->operand.if_op.else_op = opptr;
	    opptr->operand.if_op.ifop_op = conditional_ops[cond_op_ptr - 1];
            opptr->block_addr = block_addr++;
	    break;
	case OP_ENDIF:
	    if (cond_op_ptr == 0) {
		tokerror(opptr->tok, "'endif' operation with no preceding 'if'\n");
	    }
	    conditional_ops[--cond_op_ptr]->operand.if_op.endif_op = opptr;
            opptr->block_addr = block_addr++;
	    break;
	case OP_ENDIFS:
	    if (cond_op_ptr == 0) {
		tokerror(opptr->tok, "'endifs' operation with no preceding 'if'\n");
	    }
	    for (i = 0; i < cond_op_ptr; ++i) {
		conditional_ops[i]->operand.if_op.endif_op = opptr;
	    }
	    opptr->block_addr = block_addr++;
	    cond_op_ptr = 0;
	    break;
	case OP_DO:
	    if (do_op_ptr >= MAX_NESTED_BRANCHES) {
		tokerror(opptr->tok, "Too many nested do branches\n");
	    }
	    do_ops[do_op_ptr++] = opptr;
            opptr->block_addr = block_addr++;
	    break;
	case OP_LOOP_PLUS:
	case OP_LOOP:
	    if (do_op_ptr == 0) {
		tokerror(opptr->tok, "'loop' operation with no preceding 'do'\n");
	    }
	    opptr->operand.doloop_op.do_op = do_ops[--do_op_ptr];
	    break;
	case OP_I:
	    if (do_op_ptr == 0) {
		tokerror(opptr->tok, "'i' word with no preceding 'do'\n");
	    }
	    opptr->operand.doloop_op.do_op = do_ops[do_op_ptr - 1];
	    break;
	case OP_BEGIN:
	    if (begin_op_ptr == MAX_NESTED_BRANCHES) {
		tokerror(opptr->tok, "Too many nested 'begin' branches'\n");
	    }
	    begin_ops[begin_op_ptr++] = opptr;
	    opptr->block_addr = block_addr++;
	    opptr->operand.indef_op.while_op = NULL;
	    opptr->operand.indef_op.repeat_op = NULL;
	    opptr->operand.indef_op.until_op = NULL;
	    break;
	case OP_WHILE:
	    if (while_op_ptr == MAX_NESTED_BRANCHES) {
		tokerror(opptr->tok, "Too many nested 'while' branches'\n");
	    } else if (begin_op_ptr == 0 || begin_op_ptr != while_op_ptr + 1) {
		tokerror(opptr->tok, "'while' word without preceding 'begin'\n");
	    }
	    while_ops[while_op_ptr++] = opptr;
	    opptr->operand.indef_op.begin_op = begin_ops[begin_op_ptr - 1];
	    begin_ops[begin_op_ptr - 1]->operand.indef_op.while_op = opptr;
	    break;
	case OP_REPEAT:
	    if (while_op_ptr == 0) {
		tokerror(opptr->tok, "'repeat' word without preceding 'while'\n");
	    } else if (begin_op_ptr == 0) {
		tokerror(opptr->tok, "'repeat' word without preceding 'begin'\n");
	    }
	    while_ops[while_op_ptr - 1]->operand.indef_op.repeat_op = opptr;
	    opptr->operand.indef_op.while_op = while_ops[--while_op_ptr];
	    opptr->operand.indef_op.begin_op = begin_ops[--begin_op_ptr];
	    opptr->block_addr = block_addr++;
	    break;
	case OP_UNTIL:
	    if (begin_op_ptr == 0) {
		tokerror(opptr->tok, "'until' word without preceding 'begin'\n");
	    } else if (begin_ops[begin_op_ptr - 1]->operand.indef_op.while_op != NULL) {
		tokerror(opptr->tok, "'until' word used with 'while' word\n");
	    }
	    begin_ops[begin_op_ptr - 1]->operand.indef_op.until_op = opptr;
	    opptr->operand.indef_op.begin_op = begin_ops[--begin_op_ptr];
	    opptr->block_addr = block_addr++;
	    break;
	case OP_LEAVE:
	    if (begin_op_ptr == 0) {
		tokerror(opptr->tok, "'leave' word used out of a loop\n");
	    }
	    if (begin_ops[begin_op_ptr - 1]->operand.indef_op.while_op != NULL) {
		assert(while_op_ptr > 0);
		opptr->operand.indef_op.leave_jump = &(while_ops[while_op_ptr - 1]->operand.indef_op.repeat_op);
	    } else {
		opptr->operand.indef_op.leave_jump = &(begin_ops[begin_op_ptr - 1]->operand.indef_op.until_op);
	    }
	    break;
	default:
	    break;
	}
	opptr = opptr->next;
    }
    if (cond_op_ptr != 0) {
	tokerror(conditional_ops[0]->tok, "Conditional beginning here not balanced\n");
    }
    if (begin_op_ptr != 0) {
	tokerror(begin_ops[0]->tok, "Indefinite loop beginning here not closed by 'repeat' or 'until'\n");
    }
}
