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

#define COMMENT_CHAR '\\'

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
    if (*p == COMMENT_CHAR) {
	while (!END_OF_FILE(p) && *p != '\n') {
	    update_file_position(*p);
	    p++;
	}
	goto again;
    }
    new_token->pos.file = lex_file_name;
    new_token->pos.row = file_row;
    new_token->pos.col = file_col;
    while (!END_OF_FILE(p) && s - new_token->text < MAX_TOKEN_LENGTH && !isspace(*p)) {
	update_file_position(*p);
	*s++ = *p;
	p++;
    }
    file_offset = p - lex_file;
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
