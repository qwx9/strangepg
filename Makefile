PROGRAM:= strangepg
VERSION:= 0.8.4
BINTARGET:= $(PROGRAM)
ALLTARGETS:=\
	$(BINTARGET)\

DIRS:=\
	strawk\

#PREFIX:= /usr/local
PREFIX?= $(HOME)/.local
BINDIR:= $(PREFIX)/bin

OBJS:=\
	sokol/draw.o\
	sokol/ui.o\
	lib/chan.o\
	lib/flextgl/flextGL.o\
	lib/queue.o\
	lib/plan9/getfields.o\
	lib/plan9/seprint.o\
	lib/plan9/strecpy.o\
	linux/awk.o\
	linux/fs.o\
	linux/sys.o\
	linux/threads.o\
	cmd/awkprog.o\
	cmd/cmd.o\
	draw/color.o\
	draw/draw.o\
	fs/em.o\
	fs/fs.o\
	fs/gfa.o\
	fs/index.o\
	fs/layout.o\
	fs/load.o\
	fs/metacsv.o\
	graph/graph.o\
	layout/bo.o\
	layout/circ.o\
	layout/conga.o\
	layout/fr.o\
	layout/layout.o\
	layout/linear.o\
	layout/pfr.o\
	layout/random.o\
	ui/ui.o\
	util/print.o\
	strpg.o\
	glsl/node.vert.o\
	glsl/node.frag.o\
	glsl/edge.vert.o\
	glsl/edge.frag.o\
	glsl/scr.vert.o\
	glsl/scr.frag.o\
	glsl/bezier.vert.o\
	glsl/bezier.tcs.o\
	glsl/bezier.tes.o\
	glsl/bezier.geom.o\
	glsl/bezier.frag.o\

COARSENOBJS:=\
	coarsen.o\
	fs/em.o\
	fs/fs.o\
	lib/chan.o\
	lib/queue.o\
	linux/fs.o\
	linux/sys.o\
	linux/threads.o\
	util/print.o\

DEPS:=$(patsubst %.o,%.d,$(OBJS) $(COARSENOBJS))

ALLOBJS:=\
	$(OBJS)\
	$(COARSENOBJS)\

CLEANFILES:=\
	$(OBJS)\
	$(COARSENOBJS)\
	$(DEPS)\

ifeq ($(wildcard .git),.git)
	VERSION:= $(shell git describe --tags)
	GITCMD:= git describe --abbrev=8 --always
	ifneq ($(shell git diff-index --name-only HEAD --),"")
		GITCMD+= --dirty
	endif
	GIT_HEAD:= $(shell $(GITCMD))
	ifneq ($(GIT_HEAD),)
		VERSION+= git_$(GIT_HEAD)
	endif
endif

CC?= clang
OFLAGS?= -O3 -pipe -march=native
CFLAGS?= $(OFLAGS)
# doesn't even work, what bullshit
CFLAGS+= -fextended-identifiers -finput-charset=UTF-8
# _XOPEN_SOURCE: M_PI et al
# _POSIX_C_SOURCE >= 200809L: getline (in _DEFAULT_SOURCE)
CFLAGS+= -D_XOPEN_SOURCE=500
CFLAGS+= -pthread
# generate dependency files for headers
CFLAGS+= -MMD -MP
CFLAGS+= -DVERSION="\"$(VERSION)\""
WFLAGS?= -Wall -Wformat=2 -Wunused -Wno-parentheses
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
LDLIBS+= -lGL -lX11 -lXcursor -lXi -lm

ifdef DEBUG
	export LLVM_PROFILE_FILE :=./llvm_%p.prof
	WFLAGS+= -Waggregate-return -Wcast-align -Wcast-qual \
			 -Wdisabled-optimization -Wfloat-equal -Winit-self -Winline \
			 -Winvalid-pch -Wunsafe-loop-optimizations \
			 -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector \
			 -Wsuggest-attribute=const -Wswitch-default \
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
			-Wno-format-nonliteral -Wno-int-to-void-pointer-cast \
			-Wno-implicit-fallthrough
endif

all:	$(ALLTARGETS) dirall

# FIXME
strindex:	index.sh
	cp -x $^ $@

$(BINTARGET):	$(OBJS)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

strcoarse:	$(COARSENOBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

install: $(ALLTARGETS) dirinstall
	test -d $(BINDIR) || install -d -m755 $(BINDIR)
	for i in $(ALLTARGETS); do \
		install -m755 $$i $(BINDIR); \
	done

uninstall: $(ALLTARGETS)
	for i in $(ALLTARGETS); do \
		$(RM) $(BINDIR)/$$i; \
	done
	for i in $(DIRS); do \
		cd $$i; \
		make uninstall; \
		cd ..; \
	done

dirall:
	for i in $(DIRS); do \
		cd $$i; \
		make $(MAKEFLAGS) all; \
		cd ..; \
	done

dirinstall:
	for i in $(DIRS); do \
		cd $$i; \
		make $(MAKEFLAGS) PREFIX=$(PREFIX) install; \
		cd ..; \
	done

clean:
	$(RM) $(CLEANFILES)
	for i in $(DIRS); do \
		cd $$i; \
		make clean; \
		cd ..; \
	done

-include $(DEPS)
