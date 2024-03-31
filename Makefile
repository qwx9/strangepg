PROGRAM:= strpg
BINTARGET:= $(PROGRAM)
ALLTARGETS:=\
	$(BINTARGET)\
	coarsen\

DIRS:=\
	strawk\

#INSTALLPREFIX:= /usr/local
INSTALLPREFIX:= $(HOME)
BINDIR:= $(INSTALLPREFIX)/bin

OBJS:=\
	lib/chan.o\
	lib/queue.o\
	lib/plan9/geom.o\
	lib/plan9/getfields.o\
	lib/plan9/nrand.o\
	lib/plan9/seprint.o\
	lib/plan9/strecpy.o\
	linux/awk.o\
	linux/fs.o\
	linux/sys.o\
	linux/threads.o\
	sokol/flextgl/flextGL.o\
	sokol/draw.o\
	sokol/ui.o\
	cmd/cmd.o\
	draw/color.o\
	draw/draw.o\
	fs/em.o\
	fs/fs.o\
	fs/gfa.o\
	fs/index.o\
	fs/load.o\
	fs/meta.o\
	graph/graph.o\
	graph/vertex.o\
	layout/conga.o\
	layout/force.o\
	layout/fr.o\
	layout/layout.o\
	layout/random.o\
	rend/rend.o\
	ui/ui.o\
	util/print.o\
	strpg.o\

COARSENOBJS:=\
	coarsen.o\
	fs/em.o\
	fs/fs.o\
	lib/chan.o\
	lib/plan9/getfields.o\
	lib/plan9/nrand.o\
	lib/queue.o\
	linux/fs.o\
	linux/sys.o\
	linux/threads.o\
	util/print.o\

ALLOBJS:=\
	$(OBJS)\
	$(COARSENOBJS)\

CC= clang
OFLAGS?= -O3 -pipe -march=native
CFLAGS?= $(OFLAGS)
# doesn't even work, what bullshit
CFLAGS+= -fextended-identifiers -finput-charset=UTF-8
# _XOPEN_SOURCE: M_PI et al
# _POSIX_C_SOURCE >= 200809L: getline (in _DEFAULT_SOURCE)
CFLAGS+= -D_XOPEN_SOURCE=500
CFLAGS+= -pthread
WFLAGS?= -Wall -Wextra -Wformat=2 -Wno-parentheses
SFLAGS?= -std=c99
IFLAGS?=\
	-I.\
	-Icmd\
	-Idraw\
	-Ifs\
	-Igraph\
	-Ilayout\
	-Ilinux\
	-Irend\
	-Iui\
	-Iutil\

CFLAGS+= $(SFLAGS) $(IFLAGS) $(WFLAGS)
LDFLAGS?=
LDLIBS?= -lGL -lglfw -lm

ifdef DEBUG
	export LLVM_PROFILE_FILE :=./llvm_%p.prof
	WFLAGS+= -Waggregate-return -Wcast-align -Wcast-qual \
			 -Wdisabled-optimization -Wfloat-equal -Winit-self -Winline \
			 -Winvalid-pch -Wunsafe-loop-optimizations \
			 -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector \
			 -Wsuggest-attribute=const -Wswitch-default -Wunused \
			 -Wvariadic-macros
	CFLAGS+= -g -glldb -O0 -fprofile-instr-generate -fcoverage-mapping
else
	# c2x for omitting parameter names in a function definition
	# gnu designator: plan9 extension: struct dicks = {[enum1] {..}, [enum2] {..}}
	WFLAGS+= -Wno-unknown-pragmas -Wno-unused-value \
			-Wno-unused-function -Wno-unused-parameter \
			-Wno-unused-variable -Wno-sign-compare \
			-Wno-ignored-qualifiers \
			-Wno-c2x-extensions -Wno-gnu-designator \
			-Wno-incompatible-pointer-types-discards-qualifiers \
			-Wno-format-nonliteral -Wno-int-to-void-pointer-cast
endif
ifdef STATIC
	LDFLAGS+= -static
	LDLIBS+= -ldl -lpthread
endif

ifeq ($(wildcard .git),.git)
	GITCMD:= git describe --abbrev=8 --always
	ifneq ($(shell git diff-index --name-only HEAD --),"")
		GITCMD+= --dirty
	endif
	GIT_HEAD:= $(shell $(GITCMD))
	ifneq ($(GIT_HEAD),)
		VERSION:= git_$(GIT_HEAD)
	endif
endif

all:	$(ALLTARGETS) dirall /tmp/main.awk

/tmp/main.awk: cmd/main.awk
	cp -x $^ $@

$(BINTARGET):	$(OBJS)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

coarsen:	$(COARSENOBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

install: $(ALLTARGETS) dirinstall
	test -d $(BINDIR) || install -d -m755 $(BINDIR)
	for i in $(ALLTARGETS); do \
		install -m755 $$i $(BINDIR); \
	done

uninstall: $(ALLTARGETS)
	for i in $(ALLTARGETS); do \
		rm -f $(BINDIR)/$$i; \
	done
	for i in $(DIRS); do \
		cd $$i; \
		make uninstall; \
		cd ..; \
	done

dirall:
	for i in $(DIRS); do \
		cd $$i; \
		make all; \
		cd ..; \
	done

dirinstall:
	for i in $(DIRS); do \
		cd $$i; \
		make install; \
		cd ..; \
	done

clean:
	rm -f $(ALLOBJS) $(ALLTARGETS)
	for i in $(DIRS); do \
		cd $$i; \
		make clean; \
		cd ..; \
	done
