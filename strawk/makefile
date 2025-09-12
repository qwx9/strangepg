PREFIX?= $(HOME)/.local
BINDIR:= $(PREFIX)/bin
PROGRAM:= strawk
BINTARGET:= $(PROGRAM)
ALLTARGETS:= $(BINTARGET)
OFILES = awkgram.tab.o b.o main.o parse.o proctab.o tran.o lib.o run.o lex.o mt19937-64.o alloc.o unix.o
DEPS:=$(patsubst %.o,%.d,$(OFILES))
CLEANFILES:=\
	$(BINTARGET)\
	$(OFILES)\
	$(DEPS)\

YACC:= bison
YFLAGS:= -d
CC?= clang
CPPFLAGS+= -MMD -MP
CFLAGS?= -O2 -march=native -pipe
LDFLAGS?=
LDLIBS:= -lm

ifdef DEBUG
	ifdef ASAN
		CFLAGS+= -O1 -g -Wall -pedantic -Wcast-qual -Wno-parentheses
		CPPFLAGS+= -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS -fexceptions
		CFLAGS+= -fasynchronous-unwind-tables
		CFLAGS+= -fstack-clash-protection -fstack-protector-strong
		CFLAGS+= -fsanitize=address -fsanitize=leak -fsanitize=undefined \
				 -fsanitize-address-use-after-scope
		LDFLAGS+= -fsanitize=address -fsanitize=leak -fsanitize=undefined \
				  -fsanitize-address-use-after-scope
	else
		CFLAGS+= -O0 -pg -g -Wall -pedantic -Wcast-qual -Wno-parentheses
	endif
	ifdef PROFILE
		CFLAGS+= -pg
		LDFLAGS+= -pg
	endif
	ifeq ($(CC), clang)
		CFLAGS+= -glldb
		export LLVM_PROFILE_FILE:= ./llvm_%p.prof
		CFLAGS+= -fprofile-instr-generate -fcoverage-mapping
	endif
else
	CFLAGS+= -flto
	CFLAGS+= -Wall -pedantic -Wno-parentheses -Wno-format-pedantic
	CPPFLAGS+= -DNDEBUG
	LDFLAGS+= -flto
endif

all: $(BINTARGET)

$(OFILES): awkgram.tab.h

$(BINTARGET): $(OFILES)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

awkgram.tab.c awkgram.tab.h: awk.h proto.h awkgram.y
	if command -v $(YACC) >/dev/null; then \
		$(YACC) $(YFLAGS) awkgram.y; \
	fi

proctab.c: maketab
	./maketab awkgram.tab.h >proctab.c

maketab: maketab.c awkgram.tab.h
	$(CC) maketab.c -o $@ $(CFLAGS)

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
