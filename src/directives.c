#include <string.h>

#include "main.h"
#include "directives.h"
#include "variables.h"

struct {
    char *dirname;
    DirWord dir;
} dir_table[] = {
    { ".var", DIR_VAR},
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
	    default:
		unreachable("preprocess");
	    }
	} else {
	    prev_tok = tok;
	    tok = tok->next;
	    prev_tok->next = tok;
	}
    }
    return newhead;
}
