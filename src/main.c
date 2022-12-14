#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "main.h"
#include "lex.h"
#include "ops.h"
#include "simulate.h"
#include "branches.h"
#include "directives.h"
#include "hash.h"
#include "variables.h"

#define OPTSTRING "+:hscvI:ra:l:"

#define MAX_INCLUDE_PATHS 100

extern char *assembler;
extern char *linker;

enum {
    SIMULATE,
    COMPILE,
};

static int action = -1;

char *input_file;     
off_t input_file_size;
char *input_file_name;

bool verbose = false;
bool run_after_compilation = false;

char *include_paths[MAX_INCLUDE_PATHS];
int nr_include_paths;

void compile(void);

static void parse_cmdline(int argc, char *argv[]);
static void init(void);
static void fini(void);
static void add_include_path(char *path);

static void add_include_path(char *path)
{
    if (nr_include_paths >= MAX_INCLUDE_PATHS) {
	tell_user(stderr, "Maximum number of include paths exceeded. What are you doing?\n");
	exit(EXIT_FAILURE);
    }
    include_paths[nr_include_paths++] = realpath(path, NULL);
}

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
    tell_user(stderr, "\t\t-h\t\tPrint this help and exit\n");
    tell_user(stderr, "\t\t-s\t\tinterpret the input file\n");
    tell_user(stderr, "\t\t-c\t\tCompile the input file\n");
    tell_user(stderr, "\t\t-v\t\tBe verbose\n");
    tell_user(stderr, "\t\t-I <DIR>\tAdd a directory to the search path for including files\n");
    tell_user(stderr, "\t\t-a <CMD>\t\tUse <CMD> as the assembler\n");
    tell_user(stderr, "\t\t-l <CMD>\t\tUse <CMD> as the linker\n");
    exit(code);
}

void parse_cmdline(int argc, char *argv[])
{
    int c, i;
   
    if (argc == 1) {
	tell_user(stderr, "No arguments given.\n");
	usage(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, OPTSTRING)) > 0) {
	switch (c) {
	case 'h':
	    usage(EXIT_SUCCESS);
	    break;
	case 's':
	    action = SIMULATE;
	    break;
	case 'c':
	    action = COMPILE;
	    break;
        case 'v':
            verbose = true;
            break;
	case 'I':
	    add_include_path(optarg);
	    break;
	case 'r':
	    run_after_compilation = true;
	    break;
	case 'a':
	    assembler = optarg;
	    break;
	case 'l':
	    linker = optarg;
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
    if (input_file_name == NULL) {
	tell_user(stderr, "No input file specified.\n");
	exit(EXIT_FAILURE);
    }
    simulated_argv = &argv[optind];
    i = optind;
    while (argv[i] != NULL)
	i++;
    simulated_argc = i - optind;
}

void init(void)
{
    atexit(fini);
}

void fini(void)
{
    /* Nothing at the moment. */

}

bool simulating(void)
{
    return action == SIMULATE;
}

int main(int argc, char *argv[])
{
    parse_cmdline(argc, argv);
    init();
    init_glob_hash();
    tokens = lex(input_file_name);
    assert(tokens != NULL);
    tokens = preprocess(tokens);
    operations = tokens_to_ops(tokens);
    cross_reference_branches(operations);
    if (action == SIMULATE) {
	simulate();
    } else if (action == COMPILE) {
	compile();
    } else {
	tell_user(stderr, "No action specified\n");
	usage(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
