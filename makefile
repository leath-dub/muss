.POSIX:
.SUFFIXES:

prefix := lib/lowdown
src = autolink.c buffer.c diff.c document.c entity.c gemini.c html.c \
html_escape.c latex.c library.c libdiff.c nroff.c odt.c smartypants.c term.c \
tree.c util.c compats.c
obj = $(src:.c=.o)
cflags = -Ilib/lowdown/ -L. -lm -Wall -pedantic
ifdef DEBUG
DEFINES = -DDEBUG
DEBUGFLAG = -g$(DEBUG)
endif

muss: liblowdown.a
	$(CC) $(DEBUGFLAG) -o $@ src/main.c $(cflags) -l:$^ $(DEFINES)

liblowdown.a: $(obj)
	ar cr $@ $^

$(obj):
	$(CC) $(DEBUGFLAG) -c $(addprefix $(prefix)/, $(src))

clean:
	rm -rf $(obj)
	rm -rf liblowdown.a
	rm -rf muss

.PHONY: clean
