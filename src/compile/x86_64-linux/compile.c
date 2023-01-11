#ifdef __FreeBSD__
#include <sys/syscall.h>
#else
#ifdef __linux__
#include <syscall.h>
#else
#error "Unreachable"
#endif
#endif

#include <sys/wait.h>
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

extern char *readable_op_names[];

char *assembler = "as";
char *linker = "ld";

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
static void emit_functions(void);
static void emit_lvars(Function *f);
static void emit_return(void);

static void emit_OP_PUSH(Operation *op);             
static void emit_OP_PUSH_STR(Operation *op);  
static void emit_OP_PUSH_ADDR(Operation *op);        
static void emit_OP_PUSH_LVAR(Operation *op);        
static void emit_OP_ADD(Operation *op);              
static void emit_OP_MINUS(Operation *op);            
static void emit_OP_MULTIPLY(Operation *op);         
static void emit_OP_DIVIDE(Operation *op);           
static void emit_OP_MOD(Operation *op);              
static void emit_OP_DIVMOD(Operation *op);           
static void emit_OP_BAND(Operation *op);             
static void emit_OP_BOR(Operation *op);              
static void emit_OP_BXOR(Operation *op);             
static void emit_OP_LSHIFT(Operation *op);           
static void emit_OP_RSHIFT(Operation *op);           
static void emit_OP_NEGATE(Operation *op);           
static void emit_OP_PRINT(Operation *op);            
static void emit_OP_ABS(Operation *op);              
static void emit_OP_MIN(Operation *op);              
static void emit_OP_MAX(Operation *op);              
static void emit_OP_DROP(Operation *op);             
static void emit_OP_2DROP(Operation *op);            
static void emit_OP_DUP(Operation *op);              
static void emit_OP_2DUP(Operation *op);             
static void emit_OP_ROT(Operation *op);              
static void emit_OP_OVER(Operation *op);             
static void emit_OP_2OVER(Operation *op);            
static void emit_OP_SWAP(Operation *op);             
static void emit_OP_2SWAP(Operation *op);            
static void emit_OP_NIP(Operation *op);              
static void emit_OP_TUCK(Operation *op);             
static void emit_OP_ONEPLUS(Operation *op);          
static void emit_OP_ONEMINUS(Operation *op);         
static void emit_OP_EQUAL(Operation *op);            
static void emit_OP_0EQUAL(Operation *op);           
static void emit_OP_NEQUAL(Operation *op);           
static void emit_OP_GT(Operation *op);               
static void emit_OP_0GT(Operation *op);              
static void emit_OP_GE(Operation *op);               
static void emit_OP_LT(Operation *op);               
static void emit_OP_0LT(Operation *op);              
static void emit_OP_LE(Operation *op);               
static void emit_OP_IF(Operation *op);               
static void emit_OP_ELSE(Operation *op);             
static void emit_OP_ENDIF(Operation *op);            
static void emit_OP_ENDIFS(Operation *op);           
static void emit_OP_DO(Operation *op);               
static void emit_OP_LOOP(Operation *op);             
static void emit_OP_LOOP_PLUS(Operation *op);        
static void emit_OP_I(Operation *op);                
static void emit_OP_WHILE(Operation *op);            
static void emit_OP_BEGIN(Operation *op);            
static void emit_OP_REPEAT(Operation *op);           
static void emit_OP_UNTIL(Operation *op);            
static void emit_OP_LEAVE(Operation *op);            
static void emit_OP_SYS0(Operation *op);             
static void emit_OP_SYS1(Operation *op);             
static void emit_OP_SYS2(Operation *op);             
static void emit_OP_SYS3(Operation *op);             
static void emit_OP_SYS4(Operation *op);             
static void emit_OP_SYS5(Operation *op);             
static void emit_OP_SYS6(Operation *op);             
static void emit_OP_STORE8(Operation *op);           
static void emit_OP_STORE16(Operation *op);          
static void emit_OP_STORE32(Operation *op);          
static void emit_OP_STORE64(Operation *op);          
static void emit_OP_LOAD8(Operation *op);            
static void emit_OP_LOAD16(Operation *op);           
static void emit_OP_LOAD32(Operation *op);           
static void emit_OP_LOAD64(Operation *op);           
static void emit_OP_ARGC(Operation *op);             
static void emit_OP_ARGV(Operation *op);             
static void emit_OP_CALL(Operation *op);             
static void emit_OP_RETURN(Operation *op);           
static void emit_OP_DEF_LVAR(Operation *op);         
static void emit_OP_WRITE_LVAR(Operation *op);

typedef void (*emit_function)(Operation *op);

emit_function emit_table[] = {
    [OP_PUSH] = emit_OP_PUSH,
    [OP_PUSH_STR] = emit_OP_PUSH_STR,
    [OP_PUSH_ADDR] = emit_OP_PUSH_ADDR,
    [OP_PUSH_LVAR] = emit_OP_PUSH_LVAR,
    [OP_ADD] = emit_OP_ADD,
    [OP_MINUS] = emit_OP_MINUS,
    [OP_MULTIPLY] = emit_OP_MULTIPLY,
    [OP_DIVIDE] = emit_OP_DIVIDE,
    [OP_MOD] = emit_OP_MOD,
    [OP_DIVMOD] = emit_OP_DIVMOD,
    [OP_BAND] = emit_OP_BAND,
    [OP_BOR] = emit_OP_BOR,
    [OP_BXOR] = emit_OP_BXOR,
    [OP_LSHIFT] = emit_OP_LSHIFT,
    [OP_RSHIFT] = emit_OP_RSHIFT,
    [OP_NEGATE] = emit_OP_NEGATE,
    [OP_PRINT] = emit_OP_PRINT,
    [OP_ABS] = emit_OP_ABS,
    [OP_MIN] = emit_OP_MIN,
    [OP_MAX] = emit_OP_MAX,
    [OP_DROP] = emit_OP_DROP,
    [OP_2DROP] = emit_OP_2DROP,
    [OP_DUP] = emit_OP_DUP,
    [OP_2DUP] = emit_OP_2DUP,
    [OP_ROT] = emit_OP_ROT,
    [OP_OVER] = emit_OP_OVER,
    [OP_2OVER] = emit_OP_2OVER,
    [OP_SWAP] = emit_OP_SWAP,
    [OP_2SWAP] = emit_OP_2SWAP,
    [OP_NIP] = emit_OP_NIP,
    [OP_TUCK] = emit_OP_TUCK,
    [OP_ONEPLUS] = emit_OP_ONEPLUS,
    [OP_ONEMINUS] = emit_OP_ONEMINUS,
    [OP_EQUAL] = emit_OP_EQUAL,
    [OP_0EQUAL] = emit_OP_0EQUAL,
    [OP_NEQUAL] = emit_OP_NEQUAL,
    [OP_GT] = emit_OP_GT,
    [OP_0GT] = emit_OP_0GT,
    [OP_GE] = emit_OP_GE,
    [OP_LT] = emit_OP_LT,
    [OP_0LT] = emit_OP_0LT,
    [OP_LE] = emit_OP_LE,
    [OP_IF] = emit_OP_IF,
    [OP_ELSE] = emit_OP_ELSE,
    [OP_ENDIF] = emit_OP_ENDIF,
    [OP_ENDIFS] = emit_OP_ENDIFS,
    [OP_DO] = emit_OP_DO,
    [OP_LOOP] = emit_OP_LOOP,
    [OP_LOOP_PLUS] = emit_OP_LOOP_PLUS,
    [OP_I] = emit_OP_I,
    [OP_WHILE] = emit_OP_WHILE,
    [OP_BEGIN] = emit_OP_BEGIN,
    [OP_REPEAT] = emit_OP_REPEAT,
    [OP_UNTIL] = emit_OP_UNTIL,
    [OP_LEAVE] = emit_OP_LEAVE,
    [OP_SYS0] = emit_OP_SYS0,
    [OP_SYS1] = emit_OP_SYS1,
    [OP_SYS2] = emit_OP_SYS2,
    [OP_SYS3] = emit_OP_SYS3,
    [OP_SYS4] = emit_OP_SYS4,
    [OP_SYS5] = emit_OP_SYS5,
    [OP_SYS6] = emit_OP_SYS6,
    [OP_STORE8] = emit_OP_STORE8,
    [OP_STORE16] = emit_OP_STORE16,
    [OP_STORE32] = emit_OP_STORE32,
    [OP_STORE64] = emit_OP_STORE64,
    [OP_LOAD8] = emit_OP_LOAD8,
    [OP_LOAD16] = emit_OP_LOAD16,
    [OP_LOAD32] = emit_OP_LOAD32,
    [OP_LOAD64] = emit_OP_LOAD64,
    [OP_ARGC] = emit_OP_ARGC,
    [OP_ARGV] = emit_OP_ARGV,
    [OP_CALL] = emit_OP_CALL,
    [OP_RETURN] = emit_OP_RETURN,
    [OP_DEF_LVAR] = emit_OP_DEF_LVAR,
    [OP_WRITE_LVAR] = emit_OP_WRITE_LVAR,
};
    
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

static void emit_return(void)
{
    fprintf(asm_file, "\tmovq %%rsp, %%rax\n"
	    "\tmovq _ret_rsp, %%rsp\n"
	    "\tpopq %%rbx\n"
	    "\tmovq %%rsp, _ret_rsp\n"
	    "\tmovq %%rax, %%rsp\n"
	    "\tpushq %%rbx\n"
	    "\tret\n");
}

static void emit_lvars(Function *f)
{
    fprintf(asm_file, ".section .bss\n"
	    "%s_lvars:\n"
	    "\t.skip %d\n", f->identifier, f->lvars_count << 3);
}

static void emit_functions(void)
{
    Function *f;

    for (f = functions; f != NULL; f = f->next) {
	Operation *op;

	fprintf(asm_file, ".section .text\n");
	fprintf(asm_file, "%s:\n", f->identifier);
	fprintf(asm_file, "\tpopq %%rcx\n"
		"\tmovq %%rsp, %%rax\n"
		"\tmovq _ret_rsp, %%rsp\n"
		"\tpushq %%rcx\n"
		"\tmovq %%rsp, _ret_rsp\n"
		"\tmovq %%rax, %%rsp\n");
	for (op = f->ops; op != NULL; op = op->next) {
	    if (op->op == OP_RETURN) {
		emit_return();
	    } else {
		compile_op(op);
	    }
	}
	emit_return();
	if (f->lvars_count > 0) {
	    emit_lvars(f);
	}
    }
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
    fprintf(asm_file, "\tmovq $%d, %%rax\n"
	    "\tmovq $0, %%rdi\n"
	    "\tsyscall\n", SYS_exit);
}

static void emit_OP_PUSH(Operation *opptr)
{
    fprintf(asm_file, "\tpushq $%ld\n", opptr->operand.intr);
}

static void emit_OP_PUSH_ADDR(Operation *opptr)
{
    fprintf(asm_file, "\tpushq $%s\n", opptr->operand.variable->identifier);
}

static void emit_OP_PUSH_LVAR(Operation *opptr)
{
    assert(opptr->function != NULL);
    fprintf(asm_file, "\tmovq $%s_lvars, %%rdi\n"
	    "\taddq $%d, %%rdi\n"
	    "\tmovq (%%rdi), %%rax\n"
	    "\tpushq %%rax\n",
	    opptr->function->identifier,
	    opptr->operand.lvar->lc_num << 3);
}

static void emit_OP_PUSH_STR(Operation *opptr)
{
    fprintf(asm_file, "\tpushq $%zu\n"
	    "\tpushq $str_%d\n", opptr->operand.str.len - escape_chars(opptr), opptr->operand.str.num);
    add_to_string_pool(&opptr->operand.str);
}

static void emit_OP_ADD(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\taddq %%rax, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_MINUS(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tsubq %%rax, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_MULTIPLY(Operation *opptr)
{
    fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
	    "\tpopq %%rbx\n"
	    "\tpopq %%rax\n"
	    "\timul %%rbx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_DIVIDE(Operation *opptr)
{
    fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
	    "\tpopq %%rbx\n"
	    "\tpopq %%rax\n"
	    "\tidiv %%rbx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_MOD(Operation *opptr)
{
    fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
	    "\tpopq %%rbx\n"
	    "\tpopq %%rax\n"
	    "\tidiv %%rbx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_DIVMOD(Operation *opptr)
{
    fprintf(asm_file, "\txorq %%rdx, %%rdx\n"
	    "\tpopq %%rbx\n"
	    "\tpopq %%rax\n"
	    "\tidiv %%rbx\n"
	    "\tpushq %%rdx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_BAND(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tandq %%rax, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_BOR(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\torq %%rax, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_BXOR(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\txorq %%rax, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_LSHIFT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rcx\n"
	    "\tpopq %%rbx\n"
	    "\tshlq %%cl, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_RSHIFT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rcx\n"
	    "\tpopq %%rbx\n"
	    "\tshrq %%cl, %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_NEGATE(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tnegq %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_PRINT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rdi\n"
	    "\tcall print\n");
}

static void emit_OP_ABS(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tmovq %%rax, %%rbx\n"
	    "\tnegq %%rbx\n"
	    "\tcmovnsq %%rbx, %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_MIN(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovl %%rbx, %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_MAX(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovg %%rbx, %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_DROP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n");
}

static void emit_OP_2DROP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n");
    fprintf(asm_file, "\tpopq %%rax\n");
}

static void emit_OP_DUP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_2DUP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpushq %%rbx\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rbx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_ROT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpopq %%rcx\n"
	    "\tpushq %%rbx\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rcx\n");
}

static void emit_OP_OVER(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpushq %%rbx\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_2OVER(Operation *opptr)
{
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
}

static void emit_OP_SWAP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_2SWAP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpopq %%rcx\n"
	    "\tpopq %%rdx\n"
	    "\tpushq %%rbx\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rdx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_NIP(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_TUCK(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tpushq %%rax\n"
	    "\tpushq %%rbx\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_ONEPLUS(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tincq %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_ONEMINUS(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tdecq %%rax\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_EQUAL(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmove %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_0EQUAL(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq $0, %%rax\n"
	    "\tcmove %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_NEQUAL(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovne %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_GT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovg %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_0GT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq $0, %%rax\n"
	    "\tcmovg %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_GE(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovge %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_LT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovl %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_0LT(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq $0, %%rax\n"
	    "\tcmovl %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_LE(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq $-1, %%r10\n"
	    "\txorq %%rdx, %%rdx\n"
	    "\tcmpq %%rax, %%rbx\n"
	    "\tcmovle %%r10, %%rdx\n"
	    "\tpushq %%rdx\n");
}

static void emit_OP_IF(Operation *opptr)
{
    Operation *tmp_op;

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
}

static void emit_OP_ELSE(Operation *opptr)
{
    int endif_addr;
    Operation *tmp_op;
    
    if (opptr->operand.if_op.ifop_op == NULL) {
	tokerror(opptr->tok, "compile_op: 'else' with no preceding 'if'\n");
    }
    tmp_op = opptr->operand.if_op.ifop_op;
    endif_addr = tmp_op->operand.if_op.endif_op->block_addr;
    fprintf(asm_file, "\tjmp addr_%d\n"
	    "addr_%d:\n", endif_addr, opptr->block_addr);
}

static void emit_OP_ENDIF(Operation *opptr)
{
    emit_OP_ENDIFS(opptr);
}

static void emit_OP_ENDIFS(Operation *opptr)
{
    fprintf(asm_file, "addr_%d:\n", opptr->block_addr);
}
/*
 * VERY IMPORTANT: For OP_DO, OP_LOOP, and OP_LOOP_PLUS,
 static void emit_OP_DO(Operation *opptr)

 * registers %r14 holds the lower bound and %r15 holds
 * the upper bound - these registers are chosen because
 * they are guaranteed to be saved between function calls
 * and system calls, so as long as none of our own code

 * touches those registers we're good. This allows the OP_DO
 * operation to consume the upper and lower bounds, matching
 * the behaviour of the simulation mode.
 */

static void emit_OP_DO(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%r14\n"
	    "\tpopq %%r15\n"
	    "addr_%d:\n", opptr->block_addr);
}

static void emit_OP_LOOP(Operation *opptr)
{
    fprintf(asm_file, "\tincq %%r14\n"
	    "\tcmpq %%r14, %%r15\n"
	    "\tjne addr_%d\n", opptr->operand.doloop_op.do_op->block_addr);
}

static void emit_OP_LOOP_PLUS(Operation *opptr)
{
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
}

static void emit_OP_BEGIN(Operation *opptr)
{
    fprintf(asm_file, "addr_%d:\n", opptr->block_addr);
}

static void emit_OP_WHILE(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\ttestq %%rax, %%rax\n"
	    "\tjz addr_%d\n",
	    opptr->operand.indef_op.repeat_op->block_addr);
}

static void emit_OP_REPEAT(Operation *opptr)
{
    fprintf(asm_file, "\tjmp addr_%d\n"
	    "addr_%d:\n",
	    opptr->operand.indef_op.begin_op->block_addr,
	    opptr->block_addr);
}

static void emit_OP_UNTIL(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\ttestq %%rax, %%rax\n"
	    "\tjnz addr_%d\n",
	    opptr->operand.indef_op.begin_op->block_addr);
}

static void emit_OP_LEAVE(Operation *opptr)
{
    assert(opptr->operand.indef_op.leave_jump != NULL);
    assert(*opptr->operand.indef_op.leave_jump != NULL);
    fprintf(asm_file, "\tjmp addr_%d\n",
	    (*opptr->operand.indef_op.leave_jump)->block_addr);
}

static void emit_OP_SYS0(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tsyscall\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_SYS1(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rdi\n"
	    "\tsyscall\n"
	    "\tpush %%rax\n");
}

static void emit_OP_SYS2(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rdi\n"
	    "\tpopq %%rsi\n"
	    "\tsyscall\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_SYS3(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rdi\n"
	    "\tpopq %%rsi\n"
	    "\tpopq %%rdx\n"
	    "\tsyscall\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_SYS4(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rdi\n"
	    "\tpopq %%rsi\n"
	    "\tpopq %%rdx\n"
	    "\tpopq %%r10\n"
	    "\tsyscall\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_SYS5(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rdi\n"
	    "\tpopq %%rsi\n"
	    "\tpopq %%rdx\n"
	    "\tpopq %%r10\n"
	    "\tpopq %%r8\n"
	    "\tsyscall\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_SYS6(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rdi\n"
	    "\tpopq %%rsi\n"
	    "\tpopq %%rdx\n"
	    "\tpopq %%r10\n"
	    "\tpopq %%r8\n"
	    "\tpopq %%r9\n"
	    "\tsyscall\n"
	    "\tpushq %%rax\n");
}

static void emit_OP_STORE8(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovb %%bl, (%%rax)\n");
}

static void emit_OP_STORE16(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovw %%bx, (%%rax)\n");
}

static void emit_OP_STORE32(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovl %%ebx, (%%rax)\n");
}

static void emit_OP_STORE64(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\tpopq %%rbx\n"
	    "\tmovq %%rbx, (%%rax)\n");
}

static void emit_OP_LOAD8(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\txorq %%rbx, %%rbx\n"
	    "\tmovb (%%rax), %%bl\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_LOAD16(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\txorq %%rbx, %%rbx\n"
	    "\tmovw (%%rax), %%bx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_LOAD32(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\txorq %%rbx, %%rbx\n"
	    "\tmovl (%%rax), %%ebx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_LOAD64(Operation *opptr)
{
    fprintf(asm_file, "\tpopq %%rax\n"
	    "\txorq %%rbx, %%rbx\n"
	    "\tmovq (%%rax), %%rbx\n"
	    "\tpushq %%rbx\n");
}

static void emit_OP_I(Operation *opptr)
{
    fprintf(asm_file, "\tpushq %%r14\n");
}

static void emit_OP_ARGC(Operation *opptr)
{
    fprintf(asm_file, "\tpushq argc\n");
}

static void emit_OP_ARGV(Operation *opptr)
{
    fprintf(asm_file, "\tpushq argv\n");
}

static void emit_OP_CALL(Operation *opptr)
{
    fprintf(asm_file, "\tcall %s\n", opptr->operand.call_op.function->identifier);
}

static void emit_OP_RETURN(Operation *opptr)
{
    tokerror(opptr->tok, "'return' operation outside function\n");
}

static void emit_OP_WRITE_LVAR(Operation *opptr)
{
    emit_OP_DEF_LVAR(opptr);
}

static void emit_OP_DEF_LVAR(Operation *opptr)
{
    assert(opptr->function != NULL);
    fprintf(asm_file, "\tmovq $%s_lvars, %%rdi\n"
	    "\taddq $%d, %%rdi\n"
	    "\tpopq %%rax\n"
	    "\tmovq %%rax, (%%rdi)\n",
	    opptr->function->identifier,
	    opptr->operand.lvar->lc_num << 3);
}

static void compile_op(Operation *opptr)
{
    Operation *tmp_op;

    static_assert(OP_COUNT == 77, "compile_op: exahustive ops handling");
    static_assert(sizeof(emit_table)/sizeof(emit_table[0]) == 76, "emit_table: exhaustive ops handling");
    fprintf(asm_file, "/* OP: %s, LOC: %s:%d:%d: */\n",
	    readable_op_names[opptr->op],
	    opptr->tok->pos.file,
	    opptr->tok->pos.row,
	    opptr->tok->pos.col);
    emit_table[opptr->op](opptr);
}

static void assemble_and_link(void)
{
    char as_command[256];
    char ld_command[256];
    /*    char exe_command[PATH_MAX+3]; */

    sprintf(as_command, "%s %s -g -o %s", assembler, asm_file_name, obj_file_name);
    sprintf(ld_command, "%s %s -o %s", linker, obj_file_name, exe_file_name);
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
	exit(WEXITSTATUS(system(exe_file_name)));
    }
}

static void emit_tail(void)
{
    fprintf(asm_file, PRINT_ASM, SYS_write);
    fprintf(asm_file, "\t.section .data\n"
	    "_ret_stack:\n"
	    "\t.skip 8000\n"
	    "_ret_stack_end:\n"
	    "_ret_rsp:"
	    "\t.quad _ret_stack\n");
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
    emit_functions();
    emit_strings();
    emit_variables();
    emit_tail();
    fclose(asm_file);
    assemble_and_link();
}
