#ifndef OPS_H_
#define OPS_H_

#include <sys/types.h>

#include "lex.h"
#include "branches.h"
#include "variables.h"

typedef enum {
    OP_PUSH,
    OP_PUSH_STR,
    OP_PUSH_ADDR,
    OP_ADD,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MOD,
    OP_DIVMOD,
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
    OP_2DROP,
    OP_DUP,
    OP_2DUP,
    OP_ROT,
    OP_OVER,
    OP_2OVER,
    OP_SWAP,
    OP_2SWAP,
    OP_NIP,
    OP_TUCK,
    OP_ONEPLUS,
    OP_ONEMINUS,

    OP_EQUAL,
    OP_0EQUAL, /* 0= */
    OP_NEQUAL,
    OP_GT, /* > */
    OP_0GT, /* 0> */
    OP_GE, /* >= */
    OP_LT, /* < */
    OP_0LT, /* 0< */
    OP_LE, /* <= */

    OP_IF,
    OP_ELSE,
    OP_ENDIF,

    OP_DO,
    OP_LOOP,
    OP_LOOP_PLUS,
    OP_I,

    OP_WHILE,
    OP_BEGIN,
    OP_REPEAT,
    OP_UNTIL,

    OP_SYS0,
    OP_SYS1,
    OP_SYS2,
    OP_SYS3,
    OP_SYS4,
    OP_SYS5,
    OP_SYS6,
    
    /* Used to denote unknown operation */
    OP_UNKNOWN,
    /* UNUSED: used to count the number of operations */
    OP_COUNT
} OpWord;

typedef struct string {
    char *text;
    size_t len;
    int num;
} String;

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
	struct {
	    struct operation *begin_op;
	    struct operation *while_op;
	    struct operation *repeat_op;
	    struct operation *until_op;
	} indef_op;
	String str;
	Variable *variable;
    } operand;
    int block_addr; /* Used for generating assembly labels */
    struct operation *next;
} Operation;

extern Operation *operations;

Operation *tokens_to_ops(Token *toks);
    
#endif /* OPS_H_ */

