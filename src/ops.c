#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"
#include "ops.h"
#include "main.h"
#include "hash.h"
#include "variables.h"
#include "macros.h"

Operation *operations = NULL;

struct {
    char *opname;
    OpWord op;
} op_table[] = {
    { "", OP_PUSH},
    { "", OP_PUSH_STR},
    { "", OP_PUSH_ADDR},
    { "+", OP_ADD},
    { "-", OP_MINUS},
    { "*", OP_MULTIPLY},
    { "/", OP_DIVIDE},
    { "mod", OP_MOD},
    { "/mod", OP_DIVMOD},
    { "and", OP_BAND},
    { "or", OP_BOR},
    { "xor", OP_BXOR},
    { "lshift", OP_LSHIFT},
    { "rshift", OP_RSHIFT},
    { "negate", OP_NEGATE},
    { ".", OP_PRINT},
    { "abs", OP_ABS},
    { "min", OP_MIN},
    { "max", OP_MAX},
    { "drop", OP_DROP},
    { "2drop", OP_2DROP},
    { "dup", OP_DUP},
    { "2dup", OP_2DUP},
    { "rot", OP_ROT},
    { "over", OP_OVER},
    { "2over", OP_2OVER},
    { "swap", OP_SWAP},
    { "2swap", OP_2SWAP},
    { "nip", OP_NIP},
    { "tuck", OP_TUCK},
    { "1+", OP_ONEPLUS},
    { "1-", OP_ONEMINUS},
    
    { "=", OP_EQUAL},
    { "0=", OP_0EQUAL},
    { "<>", OP_NEQUAL},
    { ">", OP_GT},
    { "0>", OP_0GT},
    { ">=", OP_GE},
    { "<", OP_LT},
    { "0<", OP_0LT},
    { "<=", OP_LE},

    { "if", OP_IF},
    { "else", OP_ELSE},
    { "endif", OP_ENDIF},

    { "do", OP_DO},
    { "loop", OP_LOOP},
    { "+loop", OP_LOOP_PLUS},
    { "i", OP_I},
    
    { "while", OP_WHILE},
    { "begin", OP_BEGIN},
    { "repeat", OP_REPEAT},
    { "until", OP_UNTIL},

    { "syscall0", OP_SYS0},
    { "syscall1", OP_SYS1},
    { "syscall2", OP_SYS2},
    { "syscall3", OP_SYS3},
    { "syscall4", OP_SYS4},
    { "syscall5", OP_SYS5},
    { "syscall6", OP_SYS6},

    { "!8", OP_STORE8},
    { "!16", OP_STORE16},
    { "!32", OP_STORE32},
    { "!64", OP_STORE64},
    { "@8", OP_LOAD8},
    { "@16", OP_LOAD16},
    { "@32", OP_LOAD32},
    { "@64", OP_LOAD64},

    { "argc", OP_ARGC},
    { "argv", OP_ARGV},

    { "", OP_UNKNOWN},
};

static OpWord find_op_in_table(char *opname);
static Operation *newoperation(void);
static Operation *token_to_op(Token *tok);

static OpWord find_op_in_table(char *opname)
{
    for (int i = 0; i < OP_COUNT; i++) {
        if (strcmp(opname, op_table[i].opname) == 0) {
	    return op_table[i].op;
	}
    }
    return OP_UNKNOWN;
}

static Operation *newoperation(void)
{
    Operation *new_op;

    new_op = malloc(sizeof(*new_op));
    if (new_op == NULL) {
	fatal("newoperation: malloc returned NULL!");
    }
    return new_op;
}

static Operation *token_to_op(Token *tok)
{
    Operation *new_op;
    OpWord op;
    char *p;
    int str_num;
    HashEntry *entry;

    str_num = 0;
    new_op = newoperation();
    new_op->next = NULL;
    new_op->tok = tok;
    p = tok->text;
    if (tok->type == TOKEN_STR) {
	if ((new_op->operand.str.text = malloc(tok->length + 1)) == NULL) {
	    fatal("token_to_op: parsing string and malloc returned NULL!\n");
	}
	memcpy(new_op->operand.str.text, tok->str, tok->length);
	new_op->operand.str.text[tok->length] = '\0';
	new_op->operand.str.len = tok->length;
	new_op->operand.str.num = str_num++;
	new_op->op = OP_PUSH_STR;
	return new_op;
    } else if (tok->type == TOKEN_CHAR) {
	new_op->op = OP_PUSH;
	if (tok->text[0] == '\\') {
	    switch (tok->text[1]) {
	    case '\\':
	    case '\'':
	    case '\"':
		new_op->operand.intr = tok->text[1];
		break;
	    case 'n':
		new_op->operand.intr = '\n';
		break;
	    case 'a':
		new_op->operand.intr = '\a';
		break;
	    case 'b':
		new_op->operand.intr = '\b';
		break;
	    case 't':
		new_op->operand.intr = '\t';
		break;
	    case 'v':
		new_op->operand.intr = '\v';
		break;
	    case 'f':
		new_op->operand.intr = '\f';
		break;
	    case 'r':
		new_op->operand.intr = '\r';
		break;
	    default:
		tokerror(tok, "Unknown character constant\n");
		break;
	    }
	} else {
	    new_op->operand.intr = tok->text[0];
	}
	return new_op;
    } else if ((op = find_op_in_table(tok->text)) != OP_UNKNOWN) {
	new_op->op = op;
	return new_op;
    } else if (*p == '+' || *p == '-' || isdigit(*p)) {
	/* Parse number */
	if (*p == '+' || *p == '-') {
	    p++;
	}
	while (p - tok->text < tok->length) {
	    if (*p == '.') {
		not_implemented("Floating point constants");
	    }
	    if (!isdigit(*p)) {
		tokerror(tok, "Invalid numerical constant '%s'\n", tok->text);
	    }
	    p++;
	}
	new_op->operand.intr = atoi(tok->text);
	new_op->op = OP_PUSH;
	return new_op;
    } else if ((entry = variable_reference(tok)) != NULL) {
	new_op->op = OP_PUSH_ADDR;
	new_op->operand.variable = entry->ptr;
	return new_op;
    } else if ((entry = macro_reference(tok)) != NULL) {
	/*
	 * This means that a macro has been defined after it has
	 * been used.
	 */
	tokerror(tok, "Macro '%s' defined after it is used.\n"
		 "Please move the definition to the proper place.\n", tok->text);
    }
    tokerror(tok, "Unrecognised word '%s'\n", tok->text);
    return NULL;
}

Operation *tokens_to_ops(Token *toks)
{
    Token *tokptr;
    Operation *opptr, *new_op, *head;

    opptr = NULL;
    head = NULL;
    for (tokptr = toks; tokptr != NULL; tokptr = tokptr->next) {
	new_op = token_to_op(tokptr);
	if (head == NULL) {
	    head = new_op;
	    opptr = head;
	} else {
	    opptr->next = new_op;
	    opptr = opptr->next;
	}
    }
    return head;
}
