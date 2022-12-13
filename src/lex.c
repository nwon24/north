#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "main.h"
#include "lex.h"

#define END_OF_FILE(p) ((p) >= input_file + input_file_size)

#define COMMENT_CHAR '\\'

/* Linked list of tokens */
Token *tokens = NULL;

int file_row = 1;
int file_col = 1;
off_t file_offset = 0;

static Token *gettoken(void);
static Token *newtoken(void);
static void update_file_position(int c);

static void update_file_position(int c)
{
    if (c == '\n') {
	file_row++;
	file_col = 1;
    } else {
	file_col++;
    }
}

static Token *gettoken(void)
{
    char *p, *s;
    Token *new_token;

    p = input_file + file_offset;
    if (END_OF_FILE(p)) {
	return NULL;
    }
    new_token = newtoken();
    s = new_token->text;
 again:
    while (!END_OF_FILE(p) && isspace(*p)) {
	update_file_position(*p);
	p++;
    }
    if (END_OF_FILE(p)) {
	return NULL;
    }
    if (*p == COMMENT_CHAR) {
	while (!END_OF_FILE(p) && *p != '\n') {
	    update_file_position(*p);
	    p++;
	}
	goto again;
    }
    new_token->pos.file = input_file_name;
    new_token->pos.row = file_row;
    new_token->pos.col = file_col;
    while (!END_OF_FILE(p) && s - new_token->text < MAX_TOKEN_LENGTH && !isspace(*p)) {
	update_file_position(*p);
	*s++ = *p;
	p++;
    }
    file_offset = p - input_file;
    *s = '\0';
    new_token->length = s - new_token->text;
    new_token->next = NULL;
    return new_token;
}

static Token *newtoken(void)
{
    Token *new_token;

    new_token = malloc(sizeof(*new_token));
    if (new_token == NULL) {
	fatal("newtoken: malloc returned NULL!");
    }
    return new_token;
}

void lex(void)
{
    Token *tok, *ptr;

    ptr = NULL;
    while ((tok = gettoken()) != NULL) {
	if (tokens == NULL) {
	    tokens = tok;
	    ptr = tokens;
	} else {
	    ptr->next = tok;
	    ptr = ptr->next;
	}
    }
    /*    for (ptr = tokens; ptr != NULL; ptr = ptr->next) {
	printf("%s:%d:%d: %s\n", ptr->pos.file, ptr->pos.row, ptr->pos.col, ptr->text);
	} */
}

void tokerror(Token *tok, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fprintf(stderr, "ERROR:%s:%d:%d: ", tok->pos.file, tok->pos.row, tok->pos.col);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
