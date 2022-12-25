#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "main.h"
#include "variables.h"
#include "hash.h"

#define BSIZE 24

#define HASH_SIZE 100

Variable *variables = NULL;
int nr_variables = 0;

static HashTable *glob_hash_table = NULL;

static struct {
    char *name;
    VariableType type;
    int bytesize;
} vartypes[] = {
    { "i8", V_I8, 1 },
    { "i16", V_I16, 2 },
    { "i32", V_I32, 4 },
    { "i64", V_I64, 8},
    { "u8", V_U8, 1 },
    { "u16", V_U16, 2 },
    { "u32", V_U32, 4 },
    { "u64", V_U64, 8 },
    { "", V_UNKNOWN, 0 }
};

static VariableType str_to_vartype(char *str, int *bytesize);
static void malloc_var(Variable *var);

void init_glob_hash(void)
{
    glob_hash_table = new_hash_table(string_hashfn, HASH_SIZE);
}

static void realloc_variables(void)
{
    int bsize = BSIZE;

    if (variables == NULL) {
	if ((variables = malloc(bsize * sizeof(*variables))) == NULL) {
	    fatal("realloc_variables: malloc returned NULL");
	}
    } else {
	if ((variables = realloc(variables, (nr_variables + bsize) * sizeof(*variables))) == NULL) {
	    fatal("realloc_variables: realloc returned NULL");
	}
    }
}

static VariableType str_to_vartype(char *str, int *bytesize)
{
    int i = 0;

    while (vartypes[i].type != V_UNKNOWN) {
	if (strcmp(vartypes[i].name, str) == 0) {
	    *bytesize = vartypes[i].bytesize;
	    return vartypes[i].type;
	}
	i++;
    }
    return V_UNKNOWN;
}

static void malloc_var(Variable *var)
{
    var->addr = malloc(var->bytesize);
    if (var->addr == NULL) {
	fatal("malloc_var: malloc returned NULL");
    }
    memset(var->addr, 0, var->bytesize);
}

/*
 * Syntax: .var <identifier> <type> <size>
 */
Token *add_variable(Token *start)
{
    Variable *newvar;
    HashEntry *var_entry;
    Token *tok;
    int bytesize;

    if (nr_variables % BSIZE == 0) {
	realloc_variables();
    }
    newvar = &variables[nr_variables++];
    assert(strcmp(start->text, ".var") == 0);
    tok = start->next;
    /* Identifier */
    if (tok == NULL) {
	tokerror(tok, ".var directive left hanging\n");
    }
    check_identifier(tok, tok->text);
    var_entry = new_hash_entry(tok->text, newvar);
    if (add_hash_entry(glob_hash_table, var_entry) < 0) {
	tokerror(tok, "Reuse of identifier '%s'\n", tok->text);
    }
    strcpy(newvar->identifier, tok->text);

    tok = tok->next;
    /* Type */
    if (tok == NULL) {
	tokerror(tok, ".var directive left hanging\n");
    }
    bytesize = 0;
    if ((newvar->type = str_to_vartype(tok->text, &bytesize)) == V_UNKNOWN) {
	tokerror(tok, "Unknown variable type '%s'\n", tok->text);
    }
    /* Length */
    tok = tok->next;
    if (tok == NULL) {
	tokerror(tok, ".var directive left hanging\n");
    }
    if ((newvar->length = atoi(tok->text)) == 0) {
	tokerror(tok, "Invalid variable length/size '%s'\n", tok->text);
    }
    newvar->bytesize = newvar->length * bytesize;
    if (simulating() == true) {
	malloc_var(newvar);
    }
    return tok->next;
}

void check_identifier(Token *tok, char *identifier)
{
    if (strlen(identifier) > MAX_TOKEN_LENGTH) {
	tokerror(tok, "Identifier '%s' too long\n", identifier);
    }
    if (!isalpha(*identifier)) {
	tokerror(tok, "Invalid identifier name '%s' - must begin with alphabetic character\n", identifier);
    }
}

HashEntry *variable_reference(Token *tok)
{
    return in_hash(glob_hash_table, tok->text);
}
