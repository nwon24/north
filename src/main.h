#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define HERE_STR "%s:%d:%d: "

void tell_user(FILE *stream, const char *msg, ...);
void fatal(const char *msg);
void not_implemented(const char *msg);
void error(const char *msg);

#define unreachable(str) ((fatal("UNREACHABLE: " str)))

extern char *input_file;
extern off_t input_file_size;
extern char *input_file_name;

#endif /* MAIN_H_ */

