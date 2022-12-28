#ifndef MACROS_H_
#define MACROS_H_

#include "lex.h"
#include "hash.h"

/*
 * This is very generous; surely this is a practical
 * limit. This is really only to stop recursive macros
 * where one expands into another which expands into the
 * former one, leading to an infinite recursive expansion
 * that is not picked up by the conventional method of
 * detecting such stuff-ups.
 */
#define MAX_NESTED_MACROS 500

typedef struct macro {
    Token *tokens;
    Token *macro_token; /* Token with macro identifier */
    char identifier[MAX_TOKEN_LENGTH + 1];
    int size; /* Number of operations */
} Macro;

Token *add_macro(Token *start);
HashEntry *macro_reference(Token *tok);
Token *expand_macro(Macro *macro, Token *prev_tok, Token *tok);

#endif /* MACROS_H_ */
