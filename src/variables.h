#ifndef VARIABLES_H_
#define VARIABLES_H_

#include <stdlib.h>

#include "lex.h"
#include "hash.h"

typedef enum variable_type {
    V_I64,
    V_I32,
    V_I16,
    V_I8,
    V_U64,
    V_U32,
    V_U16,
    V_U8,
    V_UNKNOWN,
} VariableType;
    
typedef struct variable {
    char identifier[MAX_TOKEN_LENGTH + 1];
    VariableType type;
    int length;
    size_t bytesize;
    void *addr; /* For simulation */
} Variable;

extern Variable *variables;
extern int nr_variables;

Token *add_variable(Token *start);
HashEntry *variable_reference(Token *tok);
void init_variables_hash(void);

#endif /* VARIABLES_H_ */
