#ifndef OPS_H_
#define OPS_H_

#include <sys/types.h>

#include "lex.h"
#include "branches.h"

typedef enum {
    OP_PUSH,
    OP_ADD,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MOD,
    OP_BAND,
    OP_BOR,
    OP_BXOR,
    OP_LSHIFT,
    OP_RSHIFT,
    OP_NEGATE,
    OP_PRINT,
    OP_ABS,
    OP_MIN,
    OP_MAX,
    OP_DROP,
    OP_DUP,
    OP_2DUP,
    OP_ROT,
    OP_OVER,
    OP_SWAP,
    OP_NIP,
    OP_TUCK,

    OP_EQUAL,
    OP_NEQUAL,
    OP_GT, /* > */
    OP_GE, /* >= */
    OP_LT, /* < */
    OP_LE, /* <= */

    OP_IF,
    OP_ELSE,
    OP_ENDIF,

    OP_DO,
    OP_LOOP,
    OP_LOOP_PLUS,
    OP_I,
    /* Used to denote unknown operation */
    OP_UNKNOWN,
    /* UNUSED: used to count the number of operations */
    OP_COUNT
} OpWord;

typedef struct operation {
    OpWord op;
    Token *tok;
    union {
	ssize_t intr;
	float flt;
	struct {
            struct operation *ifop_op;
	    struct operation *else_op;
	    struct operation *endif_op;
	} if_op; /* If operation is 'if' */
	struct {
	    struct operation *do_op;
	    int start;
	    int end;
	    int current;
	} doloop_op;
    } operand;
    int block_addr; /* Used for generating assembly labels */
    struct operation *next;
} Operation;

extern Operation *operations;

Operation *tokens_to_ops(Token *toks);
    
#endif /* OPS_H_ */

