#ifndef LEX_H_
#define LEX_H_

#define MAX_TOKEN_LENGTH 14

typedef struct token {
    char text[MAX_TOKEN_LENGTH];
    int length;
    struct {
	const char *file;
	int row;
	int col;
    } pos;
    struct token *next;
} Token;

extern Token *tokens;

Token *lex(const char *file);
void tokerror(Token *tok, const char *msg, ...);

#endif /* LEX_H */
