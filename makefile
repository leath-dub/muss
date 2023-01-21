.POSIX:
.SUFFIXES:

include config.mk

prefix := lib/lowdown
src = autolink.c buffer.c diff.c document.c entity.c gemini.c html.c \
html_escape.c latex.c library.c libdiff.c nroff.c odt.c smartypants.c term.c \
tree.c util.c compats.c
cflags = $(INCS) -Wall -pedantic
ldflags = -lm
optimize = -O3

# link to static libc archive
libc := lib/musl/lib/libc.a

ifdef debug
optimize = -Og
defines = -Ddebug
debugflag = -g$(debug)
endif

all: muss options

options:
	@echo muss build options:
	@echo "CFLAGS  = $(cflags)"
	@echo "LDFLAGS = $(ldflags)"
	@echo "CC      = $(CC)"
	@echo ""
	@echo "NOTE: make sure to run 'make musl' before 'make'"

muss: liblowdown.o
	$(CC) -static $(debugflag) -c src/main.c $(cflags) $(defines) $(optimize)
	$(CC) -static $(debugflag) -o $@ main.o $^ $(ldflags) $(cflags) $(optimize) $(libc)

liblowdown.o:
	$(CC) -static $(debugflag) -r -o $@ $(addprefix $(prefix)/, $(src)) $(optimize) $(libc)

clean:
	rm -rf liblowdown.o
	rm -rf main.o
	rm -rf muss
	rm -rf libc.o

# if you want to compile only the static library
liblowdown: liblowdown.o
	ar cr -o $@.a $^

musl:
	sh sh/libc.sh

.PHONY: clean musl options
