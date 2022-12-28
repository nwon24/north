LIB = ${ARCH}/${ARCH}.a

.PHONY: all $(LIB)

all: $(LIB)

$(LIB):
	(cd ${ARCH}; make)
clean:
	(cd ${ARCH}; make clean)
