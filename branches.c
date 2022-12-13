#include "main.h"
#include "branches.h"
#include "ops.h"

void cross_reference_branches(void)
{
    Operation *opptr;
    Operation *conditional_ops[MAX_NESTED_BRANCHES];
    int nested_op_ptr;

    opptr = operations;
    nested_op_ptr = 0;
    while (opptr != NULL) {
	switch (opptr->op) {
	case  OP_IF:
	    if (nested_op_ptr >= MAX_NESTED_BRANCHES) {
		tokerror(opptr->tok, "Too many nested branches\n");
	    }
	    opptr->operand.if_op.else_op = NULL;
	    opptr->operand.if_op.endif_op = NULL;
	    conditional_ops[nested_op_ptr++] = opptr;
	    break;
	case  OP_ELSE:
	    if (nested_op_ptr == 0) {
		tokerror(opptr->tok, "'else' operation with no preceding 'if'\n");
	    }
	    opptr->operand.if_op.else_op = NULL;
	    opptr->operand.if_op.endif_op = NULL;
	    conditional_ops[nested_op_ptr-1]->operand.if_op.else_op = opptr;
	    break;
	case OP_ENDIF:
	    if (nested_op_ptr == 0) {
		tokerror(opptr->tok, "'endif' operation with no preceding 'if'\n");
	    }
	    conditional_ops[--nested_op_ptr]->operand.if_op.endif_op = opptr;
	    break;
	default:
	    break;
	}
	opptr = opptr->next;
    }
    if (nested_op_ptr != 0) {
	tokerror(conditional_ops[0]->tok, "Conditional beginning here not balanced\n");
    }
}
