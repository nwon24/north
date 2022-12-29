#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

#include "main.h"
#include "directives.h"
#include "variables.h"
#include "macros.h"
#include "hash.h"

extern char *include_paths[];
extern int nr_include_paths;

struct {
    char *dirname;
    DirWord dir;
} dir_table[] = {
    { ".var", DIR_VAR},
    { ".macro", DIR_MACRO},
    { ".endm", DIR_ENDMACRO},
    { ".include", DIR_INCLUDE},
    { "", DIR_UNKNOWN}
};

static Token *preprocess_macros(Token *tokens);
static Token *include_file(Token *tok);
static DirWord find_dir_in_table(char *dirname);

static DirWord find_dir_in_table(char *dirname)
{
    for (int i = 0; i < DIR_COUNT; i++) {
	if (strcmp(dirname, dir_table[i].dirname) == 0) {
	    return dir_table[i].dir;
	}
    }
    return DIR_UNKNOWN;
}

static Token *preprocess_macros(Token *tokens)
{
    Token *tok, *prev_tok, *newhead;
    DirWord dir;
    HashEntry *entry;
    int macro_level, include_level;

    prev_tok = NULL;
    newhead = tokens;
    macro_level = 0;
    include_level = 0;
    for (tok = tokens; tok != NULL;) {
	if ((dir = find_dir_in_table(tok->text)) == DIR_MACRO) {
	    if (prev_tok == NULL) {
		newhead = add_macro(tok);
		tok = newhead;
	    } else {
		tok = add_macro(tok);
		prev_tok->next = tok;
	    }
	} else if (dir == DIR_ENDMACRO) {
	    tokerror(tok, ".endm used without preceding .macro directive\n");
	} else if (dir == DIR_INCLUDE) {
	    if (prev_tok == NULL) {
		newhead = include_file(tok);
		tok = newhead;
	    } else {
		tok = include_file(tok);
		prev_tok->next = tok;
	    }
	    if (include_level > MAX_NESTED_INCLUDES) {
		tokerror(tok, "Maximum include files exceeded.\n");
	    } else {
		include_level++;
	    }
	} else if ((entry = macro_reference(tok)) != NULL) {
	    if (macro_level > MAX_NESTED_MACROS) {
		tokerror(tok, "Macro expansion exceeds limit of number of nested macros.\n");
	    }
	    ((Macro *)entry->ptr)->macro_token = tok;
	    tok = expand_macro(entry->ptr, prev_tok, tok);
	    if (prev_tok == NULL)
		newhead = tok;
	    if (((Macro *)entry->ptr)->macro_token != NULL) {
		macro_level++;
	    } else {
		macro_level = 0;
	    }
	} else {
	    prev_tok = tok;
	    tok = tok->next;
	    prev_tok->next = tok;
	}
    }
    return newhead;
}

static Token *include_file(Token *tok)
{
    Token *included_toks, *tmptok;
    int i;

    assert(strcmp(tok->text, ".include") == 0);
    tok = tok->next;
    if (tok == NULL) {
	tokerror(tok, "Missing file name to .include directive\n");
    }
    if (tok->type != TOKEN_STR) {
	tokerror(tok, "File to include must be string\n");
    }
    if (verbose == true)
	tell_user(stderr, "[INFO] Including file '%s'\n", tok->str);
    i = 0;
    chdir(dirname(strdup(tok->pos.file)));
    while ((included_toks = lex(tok->str)) == NULL && i < nr_include_paths) {
	chdir(include_paths[i++]);
    }
    if (included_toks == NULL)
	tokerror(tok, "Unable to include '%s'. No such file or directory in search paths.\n", tok->str);

    for (tmptok = included_toks; tmptok->next != NULL; tmptok = tmptok->next);
    tmptok->next = tok->next;
    return included_toks;
}

Token *preprocess(Token *tokens)
{
    Token *tok, *prev_tok, *newhead;
    DirWord dir;
    char oldcwd[PATH_MAX];

    prev_tok = NULL;
    getcwd(oldcwd, PATH_MAX);
    newhead = preprocess_macros(tokens);
    chdir(oldcwd);
    for (tok = newhead; tok != NULL;) {
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
