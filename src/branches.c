#include "main.h"
#include "branches.h"
#include "ops.h"

void cross_reference_branches(void)
{
    int block_addr = 0;
    Operation *opptr;
    Operation *conditional_ops[MAX_NESTED_BRANCHES];
    Operation *do_ops[MAX_NESTED_BRANCHES];
    int cond_op_ptr;
    int do_op_ptr;

    opptr = operations;
    cond_op_ptr = 0;
    do_op_ptr = 0;
    block_addr = 0;
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

	case OP_DO:
	    if (do_op_ptr >= MAX_NESTED_BRANCHES) {
		tokerror(opptr->tok, "Too many nested DO branches\n");
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
	default:
	    break;
	}
	opptr = opptr->next;
    }
    if (cond_op_ptr != 0) {
	tokerror(conditional_ops[0]->tok, "Conditional beginning here not balanced\n");
    }
}
