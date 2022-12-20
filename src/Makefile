ARCH = x86_64-linux

NAME = north
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -ggdb -MMD -MP

OBJ = main.o lex.o ops.o simulate.o branches.o strings.o
DEP = ${OBJ:.o=.d}
LIB = compile/$(ARCH)/$(ARCH).a

.PHONY: $(LIB)

all: $(NAME)
.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)
$(NAME): $(OBJ) $(LIB)
	$(CC) $(OBJ) -Lcompile/$(ARCH) -l:$(ARCH).a -o $@
$(LIB):
	(cd compile; make)

-include $(DEP)

clean:
	rm -f $(OBJ) $(NAME)
	(cd compile; make clean)
