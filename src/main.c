#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "main.h"
#include "lex.h"
#include "ops.h"
#include "simulate.h"
#include "branches.h"

#define OPTSTRING ":h"

char *input_file;     
off_t input_file_size;
char *input_file_name;

static void parse_cmdline(int argc, char *argv[]);
static void init(void);
static void fini(void);

void tell_user(FILE *stream, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    vfprintf(stream, msg, args);
    va_end(args);
}

void fatal(const char *msg)
{
    tell_user(stderr, "FATAL: %s\n", msg);
    exit(EXIT_FAILURE);
}

void not_implemented(const char *msg)
{
    tell_user(stderr, "NOT IMPLEMENTED: %s\n", msg);
    exit(EXIT_FAILURE);
}

void usage(int code)
{
    tell_user(stderr, "USAGE: north [OPTIONS] [INPUT FILE]\n");
    tell_user(stderr, "\tOptions:\n");
    tell_user(stderr, "\t\t-h\tPrint this help and exit\n");
    exit(code);
}

void parse_cmdline(int argc, char *argv[])
{
    int c;
   
    if (argc == 1) {
	tell_user(stderr, "No arguments given.\n");
	usage(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, OPTSTRING)) > 0) {
	switch (c) {
	case 'h':
	    usage(EXIT_SUCCESS);
	    break;
	case ':':
	    tell_user(stderr, "Missing argument to '-%c'\n", optopt);
	    exit(EXIT_FAILURE);
	    break;
	case '?':
	    tell_user(stderr, "Unrecognised option '-%c'\n", optopt);
	    exit(EXIT_FAILURE);
	    break;
	}
    }
    input_file_name = argv[optind];
}

void init(void)
{
    int fd;
    struct stat st;

    atexit(fini);

    fd = open(input_file_name, O_RDONLY);
    fstat(fd, &st);
    if (st.st_size == 0) {
	/* Empty file, exit quietly */
	exit(EXIT_SUCCESS);
    }
    input_file = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (input_file == MAP_FAILED) {
	fatal("init: mmap failed\n");
    }
    input_file_size = st.st_size;
    close(fd);
}

void fini(void)
{

}

int main(int argc, char *argv[])
{
    parse_cmdline(argc, argv);
    init();
    lex();
    tokens_to_ops();
    cross_reference_branches();
    simulate();
    return EXIT_SUCCESS;
}
