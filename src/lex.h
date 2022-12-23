#ifndef LEX_H_
#define LEX_H_

#define MAX_TOKEN_LENGTH 31
#define MAX_CHAR_TOKEN_LENGTH 2
#define MAX_STR_TOKEN_LENGTH 4095

typedef enum token_type {
    TOKEN_WORD,
    TOKEN_STR,
    TOKEN_CHAR,
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
    struct token *next;
} Token;

extern Token *tokens;

Token *lex(const char *file);
void tokerror(Token *tok, const char *msg, ...);

#endif /* LEX_H */
