#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "main.h"
#include "hash.h"
#include "variables.h"

#define BSIZE 24

#define HASH_SIZE 100

Macro *macros = NULL;
int nr_macros = 0;

static HashTable *macros_hash_table = NULL;

void init_macros_hash(void)
{
    macros_hash_table = new_hash_table(string_hashfn, HASH_SIZE);
}

static void realloc_macros(void)
{
    int bsize = BSIZE;

    if (macros == NULL) {
	if ((macros = malloc(bsize * sizeof(*macros))) == NULL) {
	    fatal("realloc_macros: malloc returned NULL\n");
	}
    } else {
	if ((macros = realloc(macros, (nr_macros + bsize) * sizeof(*macros))) == NULL) {
	    fatal("realloc_macros: malloc returned NULL\n");
	}
    }
}

/*
 * Syntax: .macro <operations> .endm
 */
Token *add_macro(Token *start)
{
    Token *tok, *mac_tok, *mac_start;
    Macro *new_macro;
    HashEntry *macro_entry;

    assert(strcmp(start->text, ".macro") == 0);
    tok = start->next;
    if (nr_macros % BSIZE == 0) {
	realloc_macros();
    }
    new_macro = &macros[nr_macros++];
    new_macro->size = 0;
    check_identifier(tok, tok->text);
    macro_entry = new_hash_entry(tok->text, new_macro);
    macro_entry->type = HASH_MACRO;
    if (add_hash_entry(glob_hash_table, macro_entry) < 0) {
	tokerror(tok, "Reuse of identifier '%s'\n", tok->text);
    }
    strcpy(new_macro->identifier, tok->text);

    tok = tok->next;
    mac_start = NULL;
    for (mac_tok = NULL; tok != NULL && strcmp(tok->text, ".endm") != 0; tok = tok->next) {
	if (mac_tok == NULL) {
	    mac_tok = duptoken(tok);
	    mac_start = mac_tok;
	    mac_tok->next = NULL;
	} else {
	    mac_tok->next = duptoken(tok);
	    mac_tok = mac_tok->next;
	    mac_tok->next = NULL;
	}
	new_macro->size++;
    }
    if (tok == NULL) {
	tokerror(start, "Unfinished macro definition\n");
    }
    assert(mac_start != NULL);
    new_macro->tokens = mac_start;
    return tok->next;
}

HashEntry *macro_reference(Token *tok)
{
    HashEntry *entry;

    if ((entry = in_hash(glob_hash_table, tok->text)) != NULL
	&& entry->type == HASH_MACRO)
	return entry;
    return NULL;
}

Token *expand_macro(Macro *macro, Token *prev_tok, Token *tok)
{
    Token *mac_tok, *expanded_start, *expanded_tok, *ret;

    assert(strcmp(macro->identifier, tok->text) == 0);
    assert(macro->tokens != NULL);
    ret = NULL;
    expanded_start = NULL;
    for (mac_tok = macro->tokens; mac_tok != NULL; mac_tok = mac_tok->next) {
	if (expanded_start == NULL) {
	    expanded_tok = duptoken(mac_tok);
	    expanded_start = expanded_tok;
	} else {
	    expanded_tok->next = duptoken(mac_tok);
	    expanded_tok = expanded_tok->next;
	}
    }
    if (prev_tok != NULL) {
	prev_tok->next = expanded_start;
	ret = prev_tok->next;
    } else {
	ret = expanded_start;
    }
    expanded_tok->next = tok->next;
    return ret;
}
