#ifndef OPS_H_
#define OPS_H_

#include <sys/types.h>

typedef enum {
    OP_PUSH,
    OP_ADD,
    OP_PRINT,

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

