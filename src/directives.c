#include <string.h>

#include "main.h"
#include "directives.h"
#include "variables.h"
#include "macros.h"
#include "hash.h"

struct {
    char *dirname;
    DirWord dir;
} dir_table[] = {
    { ".var", DIR_VAR},
    { ".macro", DIR_MACRO},
    { ".endm", DIR_ENDMACRO},
    { "", DIR_UNKNOWN}
};

DirWord find_dir_in_table(char *dirname)
{
    for (int i = 0; i < DIR_COUNT; i++) {
	if (strcmp(dirname, dir_table[i].dirname) == 0) {
	    return dir_table[i].dir;
	}
    }
    return DIR_UNKNOWN;
}

Token *preprocess(Token *tokens)
{
    Token *tok, *prev_tok, *newhead;
    DirWord dir;
    HashEntry *entry;

    prev_tok = NULL;
    newhead = tokens;
    for (tok = tokens; tok != NULL;) {
	if ((dir = find_dir_in_table(tok->text)) != DIR_UNKNOWN) {
	    switch (dir) {
	    case DIR_VAR:
		if (prev_tok == NULL) {
		    newhead = add_variable(tok);
		    tok = newhead;
		} else {
		    tok = add_variable(tok);
		    prev_tok->next = tok;
		}
		break;
	    case DIR_MACRO:
		if (prev_tok == NULL) {
		    newhead = add_macro(tok);
		    tok = newhead;
		} else {
		    tok = add_macro(tok);
		    prev_tok->next = tok;
		}
		break;
	    case DIR_ENDMACRO:
		tokerror(tok, ".endm used without preceding .macro directive\n");
		break;
	    default:
		unreachable("preprocess");
	    }
	} else if ((entry = macro_reference(tok)) != NULL) {
	    tok = expand_macro(entry->ptr, prev_tok, tok);
	    if (prev_tok == NULL)
		newhead = tok;
	} else {
	    prev_tok = tok;
	    tok = tok->next;
	    prev_tok->next = tok;
	}
    }
    return newhead;
}
