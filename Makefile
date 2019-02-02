CC      = /usr/bin/gcc
CFLAGS  = -Wall -g
LDFLAGS =
OUTPUT = bitmap
BUILDDIR = build

OBJ = main.o

build: $(OBJ)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTPUT) $(OBJ) $(LDFLAGS)

debug: build
	gdb $(BUILDDIR)/$(OUTPUT) a.bmp

%.o: %.c
	@echo
	@echo Building $<
	@echo ==============
	@echo
	$(CC) $(CFLAGS) -c $<

all: clean build

.PHONY: clean

clean:
	rm -df  $(OBJ)
	rm -Rdf $(BUILDDIR)

run: build
	@LD_LIBRARY_PATH=../lib/  $(BUILDDIR)/$(OUTPUT) b.bmp b.txt
