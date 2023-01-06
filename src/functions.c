#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "lex.h"
#include "functions.h"
#include "main.h"
#include "hash.h"
#include "variables.h"

Function *functions = NULL;
static Function *last_function = NULL;

static Function *alloc_function(void);

static Function *alloc_function(void)
{
    Function *new;

    if ((new = malloc(sizeof(*new))) == NULL)
	fatal("alloc_function: malloc returned NULL\n");
    return new;
}

static void add_function_to_list(Function *func)
{
    if (last_function == NULL) {
	functions = func;
	last_function = functions;
    } else {
	last_function->next = func;
	last_function = last_function->next;
    }
}

/*
 * Most of this is pretty similar to 'add_macro()'.
 */
Token *add_function(Token *start)
{
    Token *tok, *prev;
    Function *new_func;
    HashEntry *func_entry;

    assert(strcmp(start->text, ".func") == 0);
    tok = start->next;
    if (tok == NULL) {
	tokerror(start, "Unfinished function definition\n");
    }
    new_func = alloc_function();
    new_func->lvars_count = 0;
    new_func->lvars_defined = false;
    new_func->hash_table = NULL;
    check_identifier(tok, tok->text);
    func_entry = new_hash_entry(tok->text, new_func);
    func_entry->type = HASH_FUNCTION;
    if (add_hash_entry(glob_hash_table, func_entry) < 0) {
	tokerror(tok, "Reuse of identifier '%s'\n", tok->text);
    }
    strcpy(new_func->identifier, tok->text);

    new_func->tokens = tok->next;
    prev = NULL;
    for (tok = tok->next; tok != NULL && strcmp(tok->text, ".endf") != 0; tok = tok->next) {
	if (strcmp(".func", tok->text) == 0)
	    tokerror(tok, "Nested functions not allowed\n");

	tok->function = new_func;
	prev = tok;
    }
    if (tok == NULL) {
	tokerror(start, "Unfinished function definition\n");
    }
    if (prev == NULL)
	new_func->tokens = NULL;
    else
	prev->next = NULL;
    add_function_to_list(new_func);
    return tok->next;
}

HashEntry *function_reference(Token *tok)
{
    HashEntry *entry;

    if ((entry = in_hash(glob_hash_table, tok->text)) != NULL
	&& entry->type == HASH_FUNCTION)
	return entry;
    return NULL;
}
