CC      = clang
CFLAGS  = -Wall
LDFLAGS = -lm
SOURCEDIR = src
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
OUTPUT = asciimap
PREFIX = /

FILE = 022.bmp
SRCS = $(wildcard $(SOURCEDIR)/*.c)
OBJT = $(SRCS:.c=.o)
OBJ  = $(OBJT:$(SOURCEDIR)/%=$(OBJDIR)/%)

.PHONY: build
build: dir $(OBJ)
	@echo [LINK] $(OBJ)
	@$(CC) $(CFLAGS) -o $(BUILDDIR)/$(OUTPUT) $(OBJ) $(LDFLAGS)

dir:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BUILDDIR)

debug: CFLAGS+= -g -D _DEBUG
debug: build;

gdb: debug
	gdb $(BUILDDIR)/$(OUTPUT)

$(OBJDIR)/%.o: $(SOURCEDIR)/%.c
	@echo [ CC ] $<
	@$(CC) $(CFLAGS) -c $< -o $@

all: clean build

.PHONY: clean
clean:
	rm -Rdf  $(OBJDIR)
	rm -Rdf $(BUILDDIR)

run: build
	$(BUILDDIR)/$(OUTPUT) $(FILE)

install: build
	@strip $(BUILDDIR)/$(OUTPUT)
	@cp $(BUILDDIR)/$(OUTPUT) $(PREFIX)/bin/$(OUTPUT)
