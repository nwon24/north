#ifndef DIRECTIVES_H_
#define DIRECTIVES_H_

#include "lex.h"

#define MAX_NESTED_INCLUDES 100

typedef enum {
    DIR_VAR,
    DIR_MACRO,
    DIR_ENDMACRO,
    DIR_INCLUDE,

    DIR_FUNC,
    DIR_ENDFUNC,
    
    DIR_UNKNOWN,
    /* UNUSED: used to count the number of directives */
    DIR_COUNT,
} DirWord;

Token *preprocess(Token *tokens);
void init_directives_hash(void);

#endif /* DIRECTIVES_H_ */
