#ifndef DIRECTIVES_H_
#define DIRECTIVES_H_

#include "lex.h"

typedef enum {
    DIR_VAR,
    DIR_MACRO,
    DIR_ENDMACRO,

    DIR_UNKNOWN,
    /* UNUSED: used to count the number of directives */
    DIR_COUNT,
} DirWord;

Token *preprocess(Token *tokens);

#endif /* DIRECTIVES_H_ */
