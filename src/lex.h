#ifndef LEX_H_
#define LEX_H_

#define BEGIN_LVARS_CHAR	'{'
#define END_LVARS_CHAR		'}'
#define BEGIN_WRITE_LVARS_CHAR	'['
#define END_WRITE_LVARS_CHAR		']'

#define MAX_TOKEN_LENGTH 31
#define MAX_CHAR_TOKEN_LENGTH 2
#define MAX_STR_TOKEN_LENGTH 4095

struct macro;

typedef enum token_type {
    TOKEN_WORD,
    TOKEN_STR,
    TOKEN_CHAR,
    TOKEN_BEGIN_LVARS,
    TOKEN_END_LVARS,
    TOKEN_BEGIN_WRITE_LVARS,
    TOKEN_END_WRITE_LVARS,
} TokenType;
    
typedef struct token {
    char text[MAX_TOKEN_LENGTH + 1]; /* Add 1 for '\0' character */
    char str[MAX_STR_TOKEN_LENGTH + 1];
    int length;
    struct {
	const char *file;
	int row;
	int col;
    } pos;
    TokenType type;
    struct macro *macro;
    struct function *function;
    struct token *next;
} Token;

extern Token *tokens;

Token *lex(const char *file);
void tokerror(Token *tok, const char *msg, ...);
Token *duptoken(Token *tok);

#endif /* LEX_H */
