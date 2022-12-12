#ifndef OPS_H_
#define OPS_H_

#include <sys/types.h>

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
    OP_ROT,
    OP_OVER,
    OP_SWAP,
    OP_NIP,
    OP_TUCK,
    /* Used to denote unknown operation */
    OP_UNKNOWN,
    /* UNUSED: used to count the number of operations */
    OP_COUNT
} OpWord;

typedef struct operation {
    OpWord op;
    union {
	ssize_t intr;
	float flt;
    } operand;
    struct operation *next;
} Operation;

extern Operation *operations;

void tokens_to_ops(void);
    
#endif /* OPS_H_ */

