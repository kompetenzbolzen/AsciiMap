CC      = /usr/bin/gcc
CFLAGS  = -Wall -g
LDFLAGS = -lm
OUTPUT = bitmap
BUILDDIR = build

FILE = 022

OBJ = main.o bitmap.o

build: $(OBJ)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTPUT) $(OBJ) $(LDFLAGS)

debug: build
	gdb $(BUILDDIR)/$(OUTPUT)

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
	$(BUILDDIR)/$(OUTPUT) $(FILE).bmp $(FILE).txt
