CC      = /usr/bin/gcc
CFLAGS  = -Wall
LDFLAGS = -lm
SOURCEDIR = src
OUTPUT = bitmap
BUILDDIR = build

FILE = 022.bmp
SRCS = $(wildcard $(SOURCEDIR)/*.c)
OBJ = $(SRCS:.c=.o)

build: $(OBJ)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTPUT) $(OBJ) $(LDFLAGS)

debug: CFLAGS+= -g -D _DEBUG
debug: build;

gdb: debug
	gdb $(BUILDDIR)/$(OUTPUT)

%.o: %.c
	@echo [ CC ] $<
	@$(CC) $(CFLAGS) -c $< -o $@

all: clean build

.PHONY: clean

clean:
	rm -df  $(OBJ)
	rm -Rdf $(BUILDDIR)

run: build
	$(BUILDDIR)/$(OUTPUT) $(FILE)
