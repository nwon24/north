ARCH = x86_64-linux

LIB = $(ARCH)/${ARCH}.a
all: $(LIB)

$(LIB):
	(cd $(ARCH); make)
clean:
	(cd $(ARCH); make clean)
