# /****************************************************************
# Copyright (C) Lucent Technologies 1997
# All Rights Reserved
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby
# granted, provided that the above copyright notice appear in all
# copies and that both that the copyright notice and this
# permission notice and warranty disclaimer appear in supporting
# documentation, and that the name Lucent Technologies or any of
# its entities not be used in advertising or publicity pertaining
# to distribution of the software without specific, written prior
# permission.
#
# LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
# IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
# SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
# THIS SOFTWARE.
# ****************************************************************/

PREFIX?= $(HOME)/.local
BINDIR:= $(PREFIX)/bin
PROGRAM:= strawk
BINTARGET:= $(PROGRAM)
ALLTARGETS:= $(BINTARGET)
YACC:= bison
YFLAGS:= -d
CC?= clang
CFLAGS?= -O2 -pipe -march=native -Wall -pedantic -Wcast-qual -Wno-parentheses
LDFLAGS:= -lm

OFILES = awkgram.tab.o b.o main.o parse.o proctab.o tran.o lib.o run.o lex.o mt19937-64.o

CLEANFILES:=\
	$(BINTARGET)\
	$(OFILES)\
	$(DEPS)\
	maketab\

all: $(BINTARGET)

$(OFILES): awk.h proto.h awkgram.y

$(BINTARGET): $(OFILES)
	$(CC) -o $(BINTARGET) $(CFLAGS) $(OFILES) $(LDFLAGS)

awkgram.tab.c awkgram.tab.h: awk.h proto.h awkgram.y
	if command -v $(YACC) >/dev/null; then \
		$(YACC) $(YFLAGS) awkgram.y; \
	fi

proctab.c: maketab awkgram.tab.h
	./maketab awkgram.tab.h >proctab.c

maketab: maketab.c
	$(CC) $(CFLAGS) maketab.c -o maketab

install: $(ALLTARGETS)
	test -d $(BINDIR) || install -d -m755 $(BINDIR)
	for i in $(ALLTARGETS); do \
		install -m755 $$i $(BINDIR); \
	done

uninstall:
	for i in $(ALLTARGETS); do \
		$(RM) $(BINDIR)/$$i; \
	done

clean:
	$(RM) $(CLEANFILES)

nuke:
	$(RM) $(CLEANFILES) maketab proctab.c awkgram.tab.*
