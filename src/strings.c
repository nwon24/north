#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "strings.h"
#include "main.h"

/*
 * Unescapes a string by converting literal
 * escape sequences to escape sequences.
 * E.g. "hello\\n" -> "hello\n"
 */
char *unescape_chars(Operation *op)
{
    char *s, *new, *t;

    if (op->op != OP_PUSH_STR)
	unreachable("escape_chars");
    s = op->operand.str.text;
    if ((new = malloc(strlen(s) + 1)) == NULL) {
	fatal("escape_chars: malloc returned NULL");
    }
    for (t = new; *s != '\0'; s++, t++) {
	if (*s == '\\') {
	    switch (*++s) {
	    case 'n':
		*t = '\n';
		break;
	    case 'v':
		*t = '\v';
		break;
	    case 'f':
		*t = '\f';
		break;
	    case 'a':
		*t = '\a';
		break;
	    case 'b':
		*t = '\b';
		break;
	    case 't':
		*t = '\t';
		break;
	    case '\\':
	    case '\?':
	    case '\'':
	    case '\"':
		*t = *s;
		break;
	    default:
		tokerror(op->tok, "Undefined escape sequence in string");
		break;
	    }
	} else {
	    *t = *s;
	}
    }
    return new;
}

/*
 * Counts the number of escape sequences in a string
 * for OP_PUSH_STR.
 * Needed becaused in unescaped string escape sequences
 * are worth two characters.
 */
int escape_chars(Operation *op)
{
    int count;
    char *s;

    for (count = 0, s = op->operand.str.text; *s != '\0'; s++) {
	if (*s == '\\') {
	    switch (*++s) {
	    case 'n':
	    case 'v':
	    case 'f':
	    case 'a':
	    case 'b':
	    case 't':
	    case '\\':
	    case '\?':
	    case '\'':
	    case '\"':
		count++;
		break;
	    default:
		tokerror(op->tok, "Undefined escape sequence in string");
		break;
	    }
	}
    }
    return count;
}
