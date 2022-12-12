#ifndef LEX_H_
#define LEX_H_

#define MAX_TOKEN_LENGTH 14

extern int file_row;
extern int file_col;

typedef struct token {
    char text[MAX_TOKEN_LENGTH];
    int length;
    struct {
	char *file;
	int row;
	int col;
    } pos;
    struct token *next;
} Token;

extern Token *tokens;

void lex(void);
void tokerror(Token *tok, const char *msg, ...);

#endif /* LEX_H */
