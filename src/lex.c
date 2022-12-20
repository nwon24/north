#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "main.h"
#include "lex.h"

#define END_OF_FILE(p) ((p) >= lex_file + lex_file_size)

#define LINE_COMMENT_CHAR '\\'
#define INLINE_COMMENT_CHAR_START '('
#define INLINE_COMMENT_CHAR_END ')'

/* Linked list of tokens */
Token *tokens = NULL;

static int file_row = 1;
static int file_col = 1;
static off_t file_offset = 0;
static char *lex_file;
static const char *lex_file_name;
static off_t lex_file_size;

static Token *gettoken(void);
static Token *newtoken(void);
static void update_file_position(int c);
static void parse_string(Token **tok, char **p);

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

    p = lex_file + file_offset;
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
    if (*p == LINE_COMMENT_CHAR) {
	while (!END_OF_FILE(p) && *p != '\n') {
	    update_file_position(*p);
	    p++;
	}
	goto again;
    } else if (*p == INLINE_COMMENT_CHAR_START) {
	/* This allows inline comments to span several lines */
	const char *incom_file;
	int incom_row;
	int incom_col;

	incom_file = lex_file_name;
	incom_row = file_row;
	incom_col = file_col;
	while (!END_OF_FILE(p) && *p != INLINE_COMMENT_CHAR_END) {
	    update_file_position(*p);
	    p++;
	}
	if (END_OF_FILE(p)) {
	    tell_user(stderr, "%s:%d:%d: Unterminated inline comment\n",
		      incom_file, incom_row, incom_col);
	    exit(EXIT_FAILURE);
	}
	p++;
	goto again;
    }
    new_token->pos.file = lex_file_name;
    new_token->pos.row = file_row;
    new_token->pos.col = file_col;
    while (!END_OF_FILE(p) && s - new_token->text < MAX_TOKEN_LENGTH && !isspace(*p)) {
	if (*p == '\"') {
	    parse_string(&new_token, &p);
	    goto out;
	}
	update_file_position(*p);
	*s++ = *p;
	p++;
    }
    *s = '\0';
    new_token->length = s - new_token->text;
    new_token->next = NULL;
    new_token->type = TOKEN_WORD;
 out:
    file_offset = p - lex_file;
    return new_token;
}

static void parse_string(Token **tok, char **p)
{
    Token *new_token;
    char *s, *t;

    new_token = *tok;
    s = *p;
    assert(**p == '\"');
    t = new_token->str;
    (*p)++;
    while (*p - s < MAX_STR_TOKEN_LENGTH && **p != '\n' && **p != '\"') {
	update_file_position(**p);
	*t++ = **p;
	(*p)++;
    }
    if (**p == '\n') {
	tell_user(stderr, "%s:%d:%d: Unterminated string literal\n",
		  lex_file_name, file_row, file_col);
	exit(EXIT_FAILURE);
    }
    assert(**p == '\"');
    (*p)++;
    update_file_position(**p);
    *t = '\0';
    new_token->length = t - new_token->str;
    new_token->next = NULL;
    new_token->type = TOKEN_STR;
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

static void lex_init_file(const char *file_path)
{
    int fd;
    struct stat st;

    fd = open(input_file_name, O_RDONLY);
    fstat(fd, &st);
    if (st.st_size == 0) {
	/* Empty file, exit quietly */
	exit(EXIT_SUCCESS);
    }
    lex_file = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (lex_file == MAP_FAILED) {
	fatal("lex_init_file: mmap failed\n");
    }
    lex_file_size = st.st_size;
    lex_file_name = file_path;
    close(fd);
    file_row = 1;
    file_col = 1;
    file_offset = 0;
}

static void lex_fini_file(void)
{
    if (munmap(lex_file, lex_file_size) < 0)
	fatal("lex_fini_file: munmap failed\n");
}

Token *lex(const char *file_path)
{
    Token *tok, *ptr, *head;

    lex_init_file(file_path);
    ptr = NULL;
    head = NULL;
    while ((tok = gettoken()) != NULL) {
	if (head == NULL) {
	    head = tok;
	    ptr = head;
	} else {
	    ptr->next = tok;
	    ptr = ptr->next;
	}
    }
    lex_fini_file();
    return head;
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
