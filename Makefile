CC = cc
CFLAGS = -std=c11 -Wall -Wextra -O2 -ggdb -MMD -MP -D_POSIX_C_SOURCE=200809L

OBJ = src/main.o src/lex.o src/ops.o src/simulate.o src/branches.o \
      src/strings.o src/hash.o src/directives.o src/variables.o src/macros.o \
      src/compile/$(ARCH)/compile.o
SRC = ${OBJ:.o=.c}
DEP = ${OBJ:.o=.d}


all: north

-include $(DEP)

north: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)
.c.o:
	$(CC) $(CFLAGS) -Isrc/ -c -o $@ $< 
clean:
	rm $(OBJ) $(DEP) north
