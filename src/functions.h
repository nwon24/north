#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "lex.h"
#include "hash.h"

struct operation;

typedef struct function {
    Token *tokens;
    Token *function_token;
    struct operation *ops;
    char identifier[MAX_TOKEN_LENGTH + 1];
} Function;

Token *add_function(Token *start);
HashEntry *function_reference(Token *tok);

#endif /* FUNCTIONS_H_ */
