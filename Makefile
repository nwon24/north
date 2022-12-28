all: north

north:
	(cd src; make CC=$(CC) ARCH=$(ARCH))
clean:
	(cd src; make clean)
