#include <assert.h>
#include <stdlib.h>

#include "main.h"
#include "variables.h"
#include "lvariables.h"
#include "hash.h"

HashEntry *lvar_reference(Token *tok)
{
    HashEntry *entry;

    if (tok->function == NULL || tok->function->hash_table == NULL)
	return NULL;
    if ((entry = in_hash(tok->function->hash_table, tok->text)) != NULL) {
	assert(entry->type == HASH_VAR);
	return entry;
    }
    return NULL;
}

static Lvariable *new_lvar(char *identifier);

static Lvariable *new_lvar(char *identifier)
{
    Lvariable *lvar;

    if ((lvar = malloc(sizeof(*lvar))) == NULL)
	fatal("new_lvar: malloc returned NULL\n");
    lvar->identifier = identifier;
    return lvar;
}

/*
 * Identifier should be already checked.
 */
Lvariable *add_lvar(Token *tok, Function *f)
{
    Lvariable *lvar;
    HashEntry *lvar_entry;

    if (f->hash_table == NULL) {
	f->hash_table = new_hash_table(string_hashfn, LVAR_HASH_SIZE);
    }
    lvar = new_lvar(tok->text);
    lvar->lc_num = f->lvars_count++;
    if ((lvar_entry = in_hash(f->hash_table, tok->text)) != NULL) {
	tokerror(tok, "Reuse of local identifier\n");
    } else if ((lvar_entry = in_hash(glob_hash_table, tok->text)) != NULL) {
	tokerror(tok, "Reuse of global identifier in local variable\n");
    }
    lvar_entry = new_hash_entry(tok->text, lvar);
    lvar_entry->type = HASH_VAR;
    add_hash_entry(f->hash_table, lvar_entry);
    return lvar;
}
