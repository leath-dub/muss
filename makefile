.POSIX:
.SUFFIXES:

include config.mk

prefix := lib
lowdown_src = autolink.c buffer.c diff.c document.c entity.c gemini.c html.c \
html_escape.c latex.c library.c libdiff.c nroff.c odt.c smartypants.c term.c \
tree.c util.c compats.c
mustach_src = mustach.c mustach-wrap.c mustach-cjson.c
cjson_src = cJSON.c
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

muss: libcjson.o libmustach.o liblowdown.o
	$(CC) -static $(debugflag) -c src/main.c $(cflags) $(defines) $(optimize)
	$(CC) -static $(debugflag) -o $@ main.o $^ $(ldflags) $(cflags) $(optimize) $(libc)

liblowdown.o:
	$(CC) -static $(debugflag) -r -o $@ $(addprefix $(prefix)/lowdown/, $(lowdown_src)) $(optimize) $(libc)

libmustach.o:
	$(CC) -static $(debuflag) -r -o $@ $(addprefix $(prefix)/mustach/, $(mustach_src)) $(optimize) $(cflags)

libcjson.o:
	mkdir -p $(prefix)/include/cjson
	cp $(prefix)/cJSON/cJSON.h $(prefix)/include/cjson
	$(CC) -static $(debuflag) -r -o $@ $(addprefix $(prefix)/cJSON/, $(cjson_src)) $(optimize)

# if you want to compile only the static libraries
liblowdown: liblowdown.o
	ar cr -o $@.a $^

libmustach: libmustach.o
	ar cr -o $@.a $^

libcjson: libcjson.o
	ar cr -o $@.a $^

musl:
	sh sh/libc.sh

clean:
	rm -rf liblowdown.o
	rm -rf libmustach.o
	rm -rf libcjson.o
	rm -rf main.o
	rm -rf muss
	rm -rf libc.o

.PHONY: all options musl clean
