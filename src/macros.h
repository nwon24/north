#ifndef MACROS_H_
#define MACROS_H_

#include "lex.h"
#include "hash.h"

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
