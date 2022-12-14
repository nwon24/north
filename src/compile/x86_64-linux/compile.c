#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "ops.h"
#include "compile.h"

static char *asm_file_name;
static char *obj_file_name;
static char *exe_file_name;
static FILE *asm_file;

static void compile_op(Operation *opptr);
static void get_file_names(void);
static void emit_header(void);

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
	    "_start:\n");
}

static void emit_exit(void)
{
    fprintf(asm_file, "movq $60, %%rax\n"
	    "movq $0, %%rdi\n"
	    "syscall\n");
}

static void compile_op(Operation *opptr)
{
    fprintf(asm_file, "/* OP: %d, LOC: %s:%d:%d: */\n",
	    opptr->op,
	    opptr->tok->pos.file,
	    opptr->tok->pos.row,
	    opptr->tok->pos.col);
    switch (opptr->op) {
    case OP_PUSH:
	fprintf(asm_file, "\tpushq $%ld\n", opptr->operand.intr);
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
    case OP_DUP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpushq %%rax\n"
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
    case OP_SWAP:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\tpushq %%rax\n"
		"\tpushq %%rbx\n");
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
    case OP_EQUAL:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmove $-1, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_NEQUAL:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovne $-1, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_GT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovg $-1, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_GE:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovge $-1, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_LT:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovl $-1, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_LE:
	fprintf(asm_file, "\tpopq %%rax\n"
		"\tpopq %%rbx\n"
		"\txorq %%rdx, %%rdx\n"
		"\tcmpq %%rax, %%rbx\n"
		"\tcmovle $-1, %%rdx\n"
		"\tpushq %%rdx\n");
	break;
    case OP_IF:
    case OP_ELSE:
    case OP_ENDIF:
    case OP_DO:
    case OP_LOOP:
    case OP_LOOP_PLUS:
    case OP_I:
	not_implemented("compile_op: conditionals and loops\n");
    default:
	break;
    }
}

static void assemble_and_link(void)
{
    char as_command[256];
    char ld_command[256];

    sprintf(as_command, "as %s -o %s", asm_file_name, obj_file_name);
    sprintf(ld_command, "ld %s -o %s", obj_file_name, exe_file_name);
    if (verbose == true) {
        printf("[ASSEMBLER] %s\n", as_command);
    }
    system(as_command);
    if (verbose == true) {
        printf("[LINKER] %s\n", ld_command);
    }
    system(ld_command);
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
    emit_tail();
    fclose(asm_file);
    assemble_and_link();
}