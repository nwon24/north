NAME = north
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -ggdb -MMD -MP

OBJ = main.o lex.o ops.o simulate.o branches.o
DEP = ${OBJ:.o=.d}

all: $(NAME)
.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)
$(NAME): $(OBJ)
	$(CC) $^ -o $@

-include $(DEP)

clean:
	rm -f $(OBJ) $(NAME)
