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

enum {
    CONST_DECIMAL = 10, /* [1-9][0-9]* */
    CONST_HEX = 16,     /* 0x[0-9a-zA-Z]* */
    CONST_OCTAL = 8,   /* 0[0-7]* */
};

Operation *operations = NULL;

static int str_num;

struct {
    char *opname;
    OpWord op;
} op_table[] = {
    { ""		, OP_PUSH	},
    { ""		, OP_PUSH_STR	},
    { ""		, OP_PUSH_LVAR	},
    { ""		, OP_PUSH_ADDR	},
    { "+"		, OP_ADD	},
    { "-"		, OP_MINUS	},
    { "*"		, OP_MULTIPLY	},
    { "/"		, OP_DIVIDE	},
    { "mod"		, OP_MOD	},
    { "/mod"		, OP_DIVMOD	},
    { "and"		, OP_BAND	},
    { "or"		, OP_BOR	},
    { "xor"		, OP_BXOR	},
    { "lshift"		, OP_LSHIFT	},
    { "rshift"		, OP_RSHIFT	},
    { "negate"		, OP_NEGATE	},
    { "."		, OP_PRINT	},
    { "abs"		, OP_ABS	},
    { "min"		, OP_MIN	},
    { "max"		, OP_MAX	},
    { "drop"		, OP_DROP	},
    { "2drop"		, OP_2DROP	},
    { "dup"		, OP_DUP	},
    { "2dup"		, OP_2DUP	},
    { "rot"		, OP_ROT	},
    { "over"		, OP_OVER	},
    { "2over"		, OP_2OVER	},
    { "swap"		, OP_SWAP	},
    { "2swap"		, OP_2SWAP	},
    { "nip"		, OP_NIP	},
    { "tuck"		, OP_TUCK	},
    { "1+"		, OP_ONEPLUS	},
    { "1-"		, OP_ONEMINUS	},
    
    { "="		, OP_EQUAL	},
    { "0="		, OP_0EQUAL	},
    { "<>"		, OP_NEQUAL	},
    { ">"		, OP_GT		},
    { "0>"		, OP_0GT	},
    { ">="		, OP_GE		},
    { "<"		, OP_LT		},
    { "0<"		, OP_0LT	},
    { "<="		, OP_LE		},

    { "if"		, OP_IF		},
    { "else"		, OP_ELSE	},
    { "endif"		, OP_ENDIF	},
    { "endifs"		, OP_ENDIFS	},

    { "do"		, OP_DO		},
    { "loop"		, OP_LOOP	},
    { "+loop"		, OP_LOOP_PLUS	},
    { "i"		, OP_I		},
    
    { "while"		, OP_WHILE	},
    { "begin"		, OP_BEGIN	},
    { "repeat"		, OP_REPEAT	},
    { "until"		, OP_UNTIL	},
    { "leave"		, OP_LEAVE	},

    { "syscall0"	, OP_SYS0	},
    { "syscall1"	, OP_SYS1	},
    { "syscall2"	, OP_SYS2	},
    { "syscall3"	, OP_SYS3	},
    { "syscall4"	, OP_SYS4	},
    { "syscall5"	, OP_SYS5	},
    { "syscall6"	, OP_SYS6	},

    { "!8"		, OP_STORE8	},
    { "!16"		, OP_STORE16	},
    { "!32"		, OP_STORE32	},
    { "!64"		, OP_STORE64	},
    { "@8"		, OP_LOAD8	},
    { "@16"		, OP_LOAD16	},
    { "@32"		, OP_LOAD32	},
    { "@64"		, OP_LOAD64	},

    { "argc"		, OP_ARGC	},
    { "argv"		, OP_ARGV	},

    { ""		, OP_CALL	},
    { "return"		, OP_RETURN	},
    
    { ""		, OP_DEF_LVAR	},

    { ""		, OP_UNKNOWN	},
};

char *readable_op_names[] = {
    [OP_PUSH] = "OP_PUSH",       
    [OP_PUSH_STR] = "OP_PUSH_STR",   
    [OP_PUSH_LVAR] = "OP_PUSH_LVAR",   
    [OP_PUSH_ADDR] = "OP_PUSH_ADDR",
    [OP_ADD] = "OP_ADD",        
    [OP_MINUS] = "OP_MINUS",      
    [OP_MULTIPLY] = "OP_MULTIPLY",   
    [OP_DIVIDE] = "OP_DIVIDE",     
    [OP_MOD] = "OP_MOD",        
    [OP_DIVMOD] = "OP_DIVMOD",     
    [OP_BAND] = "OP_BAND",       
    [OP_BOR] = "OP_BOR",        
    [OP_BXOR] = "OP_BXOR",       
    [OP_LSHIFT] = "OP_LSHIFT",     
    [OP_RSHIFT] = "OP_RSHIFT",     
    [OP_NEGATE] = "OP_NEGATE",     
    [OP_PRINT] = "OP_PRINT",      
    [OP_ABS] = "OP_ABS",        
    [OP_MIN] = "OP_MIN",        
    [OP_MAX] = "OP_MAX",        
    [OP_DROP] = "OP_DROP",       
    [OP_2DROP] = "OP_2DROP",      
    [OP_DUP] = "OP_DUP",        
    [OP_2DUP] = "OP_2DUP",       
    [OP_ROT] = "OP_ROT",        
    [OP_OVER] = "OP_OVER",       
    [OP_2OVER] = "OP_2OVER",      
    [OP_SWAP] = "OP_SWAP",       
    [OP_2SWAP] = "OP_2SWAP",      
    [OP_NIP] = "OP_NIP",        
    [OP_TUCK] = "OP_TUCK",       
    [OP_ONEPLUS] = "OP_ONEPLUS",    
    [OP_ONEMINUS] = "OP_ONEMINUS",   
    [OP_EQUAL] = "OP_EQUAL",      
    [OP_0EQUAL] = "OP_0EQUAL",     
    [OP_NEQUAL] = "OP_NEQUAL",     
    [OP_GT] = "OP_GT",         
    [OP_0GT] = "OP_0GT",        
    [OP_GE] = "OP_GE",         
    [OP_LT] = "OP_LT",         
    [OP_0LT] = "OP_0LT",        
    [OP_LE] = "OP_LE",         
    [OP_IF] = "OP_IF",         
    [OP_ELSE] = "OP_ELSE",       
    [OP_ENDIF] = "OP_ENDIF",
    [OP_ENDIFS] = "OP_ENDIFS",
    [OP_DO] = "OP_DO",         
    [OP_LOOP] = "OP_LOOP",       
    [OP_LOOP_PLUS] = "OP_LOOP_PLUS",
    [OP_I] = "OP_I",          
    [OP_WHILE] = "OP_WHILE",      
    [OP_BEGIN] = "OP_BEGIN",      
    [OP_REPEAT] = "OP_REPEAT",     
    [OP_UNTIL] = "OP_UNTIL",      
    [OP_LEAVE] = "OP_LEAVE",      
    [OP_SYS0] = "OP_SYS0",       
    [OP_SYS1] = "OP_SYS1",       
    [OP_SYS2] = "OP_SYS2",       
    [OP_SYS3] = "OP_SYS3",       
    [OP_SYS4] = "OP_SYS4",       
    [OP_SYS5] = "OP_SYS5",       
    [OP_SYS6] = "OP_SYS6",       
    [OP_STORE8] = "OP_STORE8",     
    [OP_STORE16] = "OP_STORE16",    
    [OP_STORE32] = "OP_STORE32",    
    [OP_STORE64] = "OP_STORE64",    
    [OP_LOAD8] = "OP_LOAD8",      
    [OP_LOAD16] = "OP_LOAD16",     
    [OP_LOAD32] = "OP_LOAD32",     
    [OP_LOAD64] = "OP_LOAD64",     
    [OP_ARGC] = "OP_ARGC",       
    [OP_ARGV] = "OP_ARGV",       
    [OP_CALL] = "OP_CALL",       
    [OP_RETURN] = "OP_RETURN",
    [OP_DEF_LVAR] = "OP_DEF_LVAR",
    [OP_UNKNOWN] = "OP_UNKNOWN",
};

static OpWord find_op_in_table(char *opname);
static Operation *newoperation(void);
static Operation *token_to_op(Token *tok);

static OpWord find_op_in_table(char *opname)
{
    HashEntry *entry;

    if ((entry = in_hash(glob_hash_table, opname)) != NULL && entry->type == HASH_KEYWORD)
	return *(OpWord *)entry->ptr;
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
    HashEntry *entry;
    static bool defining_lvars = false;

    new_op = newoperation();
    new_op->function = tok->function;
    new_op->next = NULL;
    new_op->tok = tok;
    p = tok->text;
    if (defining_lvars == true) {
	Token *begin_tok;
	Lvariable *lvar;
	begin_tok = tok;
	if (tok->function == NULL)
	    tokerror(tok, "Local variables allowed only in function definitions\n");
	if (tok == NULL)
	    tokerror(begin_tok, "Local variable declarations beginning here not closed\n");
	if (tok->type == TOKEN_BEGIN_LVARS) {
	    tokerror(tok, "Nesting local variable definitions not allowed\n");
	} else if (tok->type == TOKEN_END_LVARS) {
	    defining_lvars = false;
	    return NULL;
	}
	if (tok->type != TOKEN_WORD)
	    tokerror(tok, "Invalid identifier name for local variable\n");
	check_identifier(tok, tok->text);
	lvar = add_lvar(tok, tok->function);
	new_op->op = OP_DEF_LVAR;
	new_op->operand.lvar = lvar;
	return new_op;
    } else if (tok->type == TOKEN_STR) {
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
    } else if (tok->type == TOKEN_BEGIN_LVARS) {
	defining_lvars = true;
	return NULL;
    } else if ((entry = lvar_reference(tok)) != NULL) {
	Lvariable *lvar;

	lvar = entry->ptr;
	new_op->op = OP_PUSH_LVAR;
	new_op->operand.lvar = lvar;
	return new_op;
    } else if ((op = find_op_in_table(tok->text)) != OP_UNKNOWN) {
	new_op->op = op;
	return new_op;
    } else if (*p == '+' || *p == '-' || isdigit(*p)) {
	int base;

	/* Parse number */
	if (*p == '+' || *p == '-') {
	    p++;
	}
	base = CONST_DECIMAL;
	if (*p == '0')
	    base = CONST_OCTAL;
	while (p - tok->text < tok->length) {
	    if (*p == '.') {
		not_implemented("Floating point constants");
	    }
	    if (*p == 'x') {
		if ((tok->text[0] == '-' || tok->text[0] == '+') &&
		    p - tok->text == 2)
		    base = CONST_HEX;
		else if (p - tok->text == 1) {
		    base = CONST_HEX;
		} else
		    tokerror(tok, "Invalid numerical constant '%s'\n", tok->text);
	    } else if (!isdigit(*p) && (base != CONST_HEX)) {
		tokerror(tok, "Invalid numerical constant '%s'\n", tok->text);
	    } else if (!isxdigit(*p) && base == CONST_HEX) {
		tokerror(tok, "Invalid numerical constant '%s'\n", tok->text);
	    } else if ((*p < '0' || *p > '7') && base == CONST_OCTAL) {
		tokerror(tok, "Invalid octal constant '%s'\n", tok->text);
	    }
	    p++;
	}
	new_op->operand.intr = strtol(tok->text, NULL, base);
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
    } else if ((entry = function_reference(tok)) != NULL) {
	Operation *f_op, *f_head;
	Token *f_tok;
	Function *func;

	new_op->op = OP_CALL;
	new_op->operand.call_op.function = entry->ptr;
	func = entry->ptr;
	f_head = NULL;
	f_op = NULL;
	for (f_tok = func->tokens; f_tok != NULL; f_tok = f_tok->next) {
	    Operation *new_op;
	    new_op = token_to_op(f_tok);
	    if (new_op == NULL)
		continue;
	    if (f_head == NULL) {
		f_head = new_op;
		f_op = f_head;
	    } else {
		f_op->next = new_op;
		f_op = f_op->next;
	    }
	}
	f_op->next = NULL;
	func->ops = f_head;
	cross_reference_branches(func->ops);
	return new_op;
    }
    
    tokerror(tok, "Unrecognised word '%s'\n", tok->text);
    return NULL;
}

Operation *tokens_to_ops(Token *toks)
{
    Token *tokptr;
    Operation *opptr, *new_op, *head;

    static_assert(OP_COUNT == 76, "tokens_to_ops: exhausetive op handling");
    static_assert(sizeof(op_table) == OP_COUNT * sizeof(op_table[0]));
    static_assert(sizeof(readable_op_names) == OP_COUNT * sizeof(readable_op_names[0]));
    opptr = NULL;
    head = NULL;
    for (tokptr = toks; tokptr != NULL; tokptr = tokptr->next) {
	new_op = token_to_op(tokptr);
	if (new_op == NULL)
	    continue;
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

void init_keywords_hash(void)
{
    int i;

    for (i = 0; i < OP_COUNT; i++) {
	HashEntry *new;

	if (strcmp(op_table[i].opname, "") != 0) {
	    new = new_hash_entry(op_table[i].opname, &op_table[i].op);
	    new->type = HASH_KEYWORD;
	    assert(add_hash_entry(glob_hash_table, new) == 0);
	}
    }
}
