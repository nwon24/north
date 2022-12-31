#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <libgen.h>

#include "main.h"
#include "lex.h"
#include "ops.h"
#include "compile.h"
#include "strings.h"

#define BSIZE 24

static char *asm_file_name;
static char *obj_file_name;
static char *exe_file_name;
static FILE *asm_file;

static String **string_pool = NULL;
static int string_pool_size = 0;

static void compile_op(Operation *opptr);
static void get_file_names(void);
static void emit_header(void);
static void add_to_string_pool(String *str);
static void realloc_string_pool(void);
static void emit_strings(void);

static void realloc_string_pool(void)
{
    int bsize = BSIZE;
    
    if (string_pool == NULL) {
	string_pool = malloc(bsize * sizeof(String));
	if (string_pool == NULL) {
	    fatal("realloc_string_pool: malloc returned NULL!");
	}
    } else {
	string_pool = realloc(string_pool, (string_pool_size + bsize) * sizeof(String));
	if (string_pool == NULL) {
	    fatal("realloc_string_pool: realloc returned NULL!");
	}
    }
}

static void add_to_string_pool(String *str)
{
    if (string_pool_size % BSIZE == 0) {
	realloc_string_pool();
    }
    string_pool[string_pool_size++] = str;
}

static void emit_strings(void)
{
    fprintf(asm_file, ".section .rodata\n");
    for (int i = 0; i < string_pool_size; i++) {
	fprintf(asm_file, "str_%d:\n"
		"\t.asciz \"%s\"\n", string_pool[i]->num, string_pool[i]->text);
    }
}

static void emit_variables(void)
{
    fprintf(asm_file, ".section .bss\n");
    for (Variable *var = variables; var != NULL; var = var->next) {
	fprintf(asm_file, "%s:\n"
		"\t.skip %lu\n", var->identifier, var->bytesize);
    }
    /*
     * Might as well emit argc and argv as well.
     */
    fprintf(asm_file, "argc:\n\t.skip 8\nargv:\n\t.skip 8\n");
}

static void get_file_names(void)
{
    char *p, *s, *o, *e;

    /* Add a little padding just in case */
    asm_file_name = malloc(strlen(input_file_name) + 10);
    obj_file_name = malloc(strlen(input_file_name) + 10);
    exe_file_name = malloc(strlen(input_file_name) + 10);
    if (asm_file_name == NULL || obj_file_name == NULL || exe_file_name == NULL) {
	fatal("get_file_names: malloc returned NULL");
    }
    strcpy(asm_file_name, input_file_name);
    strcpy(obj_file_name, input_file_name);
    strcpy(exe_file_name, input_file_name);
    s = NULL;
    o = NULL;
    e = NULL;
    for (p = asm_file_name; *p != '\0'; p++) {
	if (*p == '/') {
	    s = p;
	}
    }
    p = (s == NULL) ? asm_file_name : s + 1;
    s = NULL;
    for (; *p != '\0'; p++) {
	if (*p == '.')
	    s = p;
    }
    if (s == NULL) {
	s = p;
	o = obj_file_name + (s - asm_file_name);
	e = exe_file_name + (s - asm_file_name);
	*s++ = '.';
	*o++ = '.';
    } else {
	o = obj_file_name + (s - asm_file_name);
	e = exe_file_name + (s - asm_file_name);
	o++;
	s++;
    }
    *s++ = 's';
    *s = '\0';
    *o++ = 'o';
    *o = '\0';
    *e = '\0';
}

static void emit_header(void)
{
    fprintf(asm_file, ".section .text\n"
	              ".global _start\n"
	    "_start:\n"
	    "\tmovq (%%rsp), %%rax\n"
	    "\tmovq %%rax, argc\n"
	    "\tmovq %%rsp, %%rax\n"
	    "\taddq $8, %%rax\n"
	    "\tmovq %%rax, argv\n");
}

static void emit_exit(void)
{
    fprintf(asm_file, "\tmovq $60, %%rax\n"
	    "\tmovq $0, %%rdi\n"
	    "\tsyscall\n");
}

static void compile_op(Operation *opptr)
{
    Operation *tmp_op;
    int endif_addr;

    fprintf(asm_file, "/* OP: %d, LOC: %s:%d:%d: */\n",
	    opptr->op,
	    opptr->tok->pos.file,
	    opptr->tok->pos.row,
	    opptr->tok->pos.col);
    switch (opptr->op) {
    case OP_PUSH:
	fprintf(asm_file, "\tpushq $%ld\n", opptr->operand.intr);
	break;
    case OP_PUSH_ADDR:
	fprintf(asm_file, "\tpushq $%s\n", opptr->operand.variable->identifier);
	break;
    case OP_PUSH_STR:
	fprintf(asm_file, "\tpushq $%zu\n"
		"\tpushq $str_%d\n", opptr->operand.str.len - escape_chars(opptr), opptr->operand.str.num);
	add_to_string_pool(&opptr->operand.str);
	break;
    case OP_ADD:
	fprintf(asm_file, "\tpopq %%rax\n"
		          "\tpopq %%rbx\n"
		"\taddq %%rax, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_MINUS:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tsubq %%rax, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_MULTIPLY:
	fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rax\n"
		"\timul %%rbx\n"
		"\tpushq %%rax\n");
	break;
    case OP_DIVIDE:
	fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rax\n"
		"\tidiv %%rbx\n"
		"\tpushq %%rax\n");
	break;
    case OP_MOD:
	fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rax\n"
		"\tidiv %%rbx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_DIVMOD:
	fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rax\n"
		"\tidiv %%rbx\n"
		"\tpushq %%rdx\n"
		"\tpushq %%rax\n");
	break;
    case OP_BAND:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tandq %%rax, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_BOR:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\torq %%rax, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_BXOR:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rax, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_LSHIFT:
	fprintf(asm_file, "\tpopq %%rcx\n"
		"\tpopq %%rbx\n"
		"\tshlq %%cl, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_RSHIFT:
	fprintf(asm_file, "\tpopq %%rcx\n"
		"\tpopq %%rbx\n"
		"\tshrq %%cl, %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_NEGATE:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tnegq %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_PRINT:
	fprintf(asm_file, "\tpopq %%rdi\n"
		"\tcall print\n");
	break;
    case OP_ABS:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tmovq %%rax, %%rbx\n"
		"\tnegq %%rbx\n"
		"\tcmovnsq %%rbx, %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_MIN:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovl %%rbx, %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_MAX:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovg %%rbx, %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_DROP:
	fprintf(asm_file, "\tpopq %%rax\n");
	break;
    case OP_2DROP:
	fprintf(asm_file, "\tpopq %%rax\n");
	fprintf(asm_file, "\tpopq %%rax\n");
	break;
    case OP_DUP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpushq %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_2DUP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n");
	break;
    case OP_ROT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rcx\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rcx\n");
	break;
    case OP_OVER:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rbx\n");
	break;
    case OP_2OVER:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rcx\n"
		"\tpopq %%rdx\n"
		"\tpushq %%rdx\n"
		"\tpushq %%rcx\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rdx\n"
		"\tpushq %%rcx\n");
	break;
    case OP_SWAP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rbx\n");
	break;
    case OP_2SWAP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpopq %%rcx\n"
		"\tpopq %%rdx\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rdx\n"
		"\tpushq %%rax\n");
	break;
    case OP_NIP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpushq %%rax\n");
	break;
    case OP_TUCK:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rbx\n"
		"\tpushq %%rax\n");
	break;
    case OP_ONEPLUS:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tincq %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_ONEMINUS:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tdecq %%rax\n"
		"\tpushq %%rax\n");
	break;
    case OP_EQUAL:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmove %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_0EQUAL:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq $0, %%rax\n"
		"\tcmove %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_NEQUAL:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovne %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_GT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovg %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_0GT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq $0, %%rax\n"
		"\tcmovg %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_GE:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovge %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_LT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovl %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_0LT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq $0, %%rax\n"
		"\tcmovl %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_LE:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq $-1, %%r10\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovle %%r10, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_IF:
	tmp_op = NULL;
        if (opptr->operand.if_op.else_op != NULL) {
            tmp_op = opptr->operand.if_op.else_op;
        } else if (opptr->operand.if_op.endif_op == NULL) {
            tokerror(opptr->tok, "'if' not terminated by 'else' or 'endif'\n");	
	} else {
	    tmp_op = opptr->operand.if_op.endif_op;
        }
        fprintf(asm_file, "\tpopq %%rax\n"
                "\ttestq %%rax,%%rax\n"
                "\tjz addr_%d\n", tmp_op->block_addr);
	break;
    case OP_ELSE:
	if (opptr->operand.if_op.ifop_op == NULL) {
	    tokerror(opptr->tok, "compile_op: 'else' with no preceding 'if'\n");
	}
	tmp_op = opptr->operand.if_op.ifop_op;
	endif_addr = tmp_op->operand.if_op.endif_op->block_addr;
	fprintf(asm_file, "\tjmp addr_%d\n"
			  "addr_%d:\n", endif_addr, opptr->block_addr);
	break;
    case OP_ENDIF:
	fprintf(asm_file, "addr_%d:\n", opptr->block_addr);
	break;
	/*
	 * VERY IMPORTANT: For OP_DO, OP_LOOP, and OP_LOOP_PLUS,
	 * registers %r14 holds the lower bound and %r15 holds
	 * the upper bound - these registers are chosen because
	 * they are guaranteed to be saved between function calls
	 * and system calls, so as long as none of our own code
	 * touches those registers we're good. This allows the OP_DO
	 * operation to consume the upper and lower bounds, matching
	 * the behaviour of the simulation mode.
	 */
    case OP_DO:
	fprintf(asm_file, "\tpopq %%r14\n"
		"\tpopq %%r15\n"
		"addr_%d:\n", opptr->block_addr);
	break;
    case OP_LOOP:
	fprintf(asm_file, "\tincq %%r14\n"
		"\tcmpq %%r14, %%r15\n"
		"\tjne addr_%d\n", opptr->operand.doloop_op.do_op->block_addr);
	break;
    case OP_LOOP_PLUS:
	fprintf(asm_file, "\tpopq %%r13\n"
		"\taddq %%r13, %%r14\n"
		"\tcmpq $0, %%r13\n"
		"\tjl 1f\n"
		"\tcmpq %%r14, %%r15\n"
		"\tjg addr_%d\n"
		"\tjmp 2f\n"
		"1:\n"
		"\tcmpq %%r14, %%r15\n"
		"\tjl addr_%d\n"
		"2:\n", opptr->operand.doloop_op.do_op->block_addr,
		opptr->operand.doloop_op.do_op->block_addr);
	break;
    case OP_BEGIN:
	fprintf(asm_file, "addr_%d:\n", opptr->block_addr);
	break;
    case OP_WHILE:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\ttestq %%rax, %%rax\n"
		"\tjz addr_%d\n",
		opptr->operand.indef_op.repeat_op->block_addr);
	break;
    case OP_REPEAT:
	fprintf(asm_file, "\tjmp addr_%d\n"
		"addr_%d:\n",
		opptr->operand.indef_op.begin_op->block_addr,
		opptr->block_addr);
	break;
    case OP_UNTIL:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\ttestq %%rax, %%rax\n"
		"\tjnz addr_%d\n",
		opptr->operand.indef_op.begin_op->block_addr);
	break;
    case OP_SYS0:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tsyscall\n"
		"\tpushq %%rax\n");
	break;
    case OP_SYS1:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rdi\n"
		"\tsyscall\n"
		"\tpush %%rax\n");
	break;
    case OP_SYS2:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rdi\n"
		"\tpopq %%rsi\n"
		"\tsyscall\n"
		"\tpushq %%rax\n");
	break;
    case OP_SYS3:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rdi\n"
		"\tpopq %%rsi\n"
		"\tpopq %%rdx\n"
		"\tsyscall\n"
		"\tpushq %%rax\n");
	break;
    case OP_SYS4:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rdi\n"
		"\tpopq %%rsi\n"
		"\tpopq %%rdx\n"
		"\tpopq %%r10\n"
		"\tsyscall\n"
		"\tpushq %%rax\n");
	break;
    case OP_SYS5:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rdi\n"
		"\tpopq %%rsi\n"
		"\tpopq %%rdx\n"
		"\tpopq %%r10\n"
		"\tpopq %%r8\n"
		"\tsyscall\n"
		"\tpushq %%rax\n");
	break;
    case OP_SYS6:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rdi\n"
		"\tpopq %%rsi\n"
		"\tpopq %%rdx\n"
		"\tpopq %%r10\n"
		"\tpopq %%r8\n"
		"\tpopq %%r9\n"
		"\tsyscall\n"
		"\tpushq %%rax\n");
	break;
    case OP_STORE8:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovb %%bl, (%%rax)\n");
	break;
    case OP_STORE16:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovw %%bx, (%%rax)\n");
	break;
    case OP_STORE32:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovl %%ebx, (%%rax)\n");
	break;
    case OP_STORE64:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tmovq %%rbx, (%%rax)\n");
	break;
    case OP_LOAD8:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\txorq %%rbx, %%rbx\n"
		"\tmovb (%%rax), %%bl\n"
		"\tpushq %%rbx\n");
	break;
    case OP_LOAD16:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\txorq %%rbx, %%rbx\n"
		"\tmovw (%%rax), %%bx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_LOAD32:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\txorq %%rbx, %%rbx\n"
		"\tmovl (%%rax), %%ebx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_LOAD64:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\txorq %%rbx, %%rbx\n"
		"\tmovq (%%rax), %%rbx\n"
		"\tpushq %%rbx\n");
	break;
    case OP_I:
	fprintf(asm_file, "\tpushq %%r14\n");
	break;
    case OP_ARGC:
	fprintf(asm_file, "\tpushq argc\n");
	break;
    case OP_ARGV:
	fprintf(asm_file, "\tpushq argv\n");
	break;
    case OP_CALL:
	not_implemented("compile: OP_CALL");
	break;
    default:
	break;
    }
}

static void assemble_and_link(void)
{
    char as_command[256];
    char ld_command[256];
    /*    char exe_command[PATH_MAX+3]; */

    sprintf(as_command, "as %s -g -o %s", asm_file_name, obj_file_name);
    sprintf(ld_command, "ld %s -o %s", obj_file_name, exe_file_name);
    if (verbose == true) {
        printf("[ASSEMBLER] %s\n", as_command);
    }
    if (system(as_command) != 0) {
	tell_user(stderr, "[INFO] Assembler failed.\n");
	exit(EXIT_FAILURE);
    }
    if (verbose == true) {
        printf("[LINKER] %s\n", ld_command);
    }
    if (system(ld_command) != 0) {
	tell_user(stderr, "[INFO] Linker failed.\n");
	exit(EXIT_FAILURE);
    }
    if (run_after_compilation == true) {
	system(exe_file_name);
    }
}

static void emit_tail(void)
{
    fprintf(asm_file, PRINT_ASM);
}

void compile(void)
{
    Operation *opptr;

    get_file_names();
    asm_file = fopen(asm_file_name, "w");
    if (asm_file == NULL) {
	tell_user(stderr, "Could not open %s for writing\n", asm_file_name);
	fatal("File access error");
    }
    emit_header();
    for (opptr = operations; opptr != NULL; opptr = opptr->next) {
	compile_op(opptr);
    }
    emit_exit();
    emit_strings();
    emit_variables();
    emit_tail();
    fclose(asm_file);
    assemble_and_link();
}
