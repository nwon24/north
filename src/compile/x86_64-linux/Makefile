CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -ggdb -MMD -MP -I../.. -D_POSIX_C_SOURCE=200809L

LIB = x86_64-linux.a

OBJ = compile.o
DEP = ${OBJ:.o=.d}

all: $(LIB)

-include $(DEP)

.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)

$(LIB): $(OBJ)
	ar rcs $@ $^

clean:
	rm -f $(OBJ) $(LIB)
