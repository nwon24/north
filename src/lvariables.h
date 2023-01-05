#ifndef LVARIABLES_H
#define LVARIABLES_H

#include "lex.h"
#include "functions.h"
#include "hash.h"
#include "simulate.h"

#define LVAR_HASH_SIZE 50

struct function;

typedef struct lvar {
    char *identifier;
    int lc_num;	/* Local number used as index */
    word val;	/* For simulation */
} Lvariable;

Lvariable *add_lvar(Token *tok, struct function *f);
HashEntry *lvar_reference(Token *tok);

#endif /* LVARIABLES_H */
