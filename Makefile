PROGRAM:= strangepg
VERSION:= 0.8.14
BINTARGET:= $(PROGRAM)
ALLTARGETS:=\
	$(BINTARGET)\

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

PREFIX?= $(HOME)/.local
BINDIR:= $(PREFIX)/bin
MAKE?= make
ifeq ($(OS),Windows_NT)
	OS:= $(shell uname 2>/dev/null || echo Win64)
	OS:= $(patsubst CYGWIN%,Cygwin,$(OS))
else
	OS:= $(shell uname 2>/dev/null || echo NEIN)
endif
ifndef MAKE
	ifeq ($(OS),OpenBSD)
		MAKE:= gmake
	else
		MAKE:= make
	endif
endif
ifndef TARGET
	ifeq ($(OS),Darwin)
		TARGET:= MacOS
	else
		TARGET:= Unix
	endif
endif

CC?= clang
#CPPFLAGS+= NDEBUG
CPPFLAGS+= -MMD -MP
CPPFLAGS+= -fextended-identifiers -finput-charset=UTF-8
CPPFLAGS+= -pthread
# _XOPEN_SOURCE: M_PI et al
# _POSIX_C_SOURCE >= 200809L: getline (in _DEFAULT_SOURCE)
CPPFLAGS+= -D_XOPEN_SOURCE=500
CPPFLAGS+= -DVERSION="\"$(VERSION)\""
CPPFLAGS+=\
	-I.\
	-Icmd\
	-Idraw\
	-Ifs\
	-Igraph\
	-Ilayout\
	-Iposix\
	-Irend\
	-Iui\
	-Iutil\

CFLAGS?= -O3 -pipe -march=native
# with minor plan9esque violations
CFLAGS+= -std=c99
CFLAGS+= -Wall -Wformat=2 -Wunused  -Wno-parentheses -Wno-unknown-pragmas
ifdef DEBUG
	export LLVM_PROFILE_FILE :=./llvm_%p.prof
	ifeq ($(CC), clang)
		CFLAGS+= -g -glldb -O0 -fprofile-instr-generate -fcoverage-mapping
	else
		CFLAGS+= -g -ggdb -O0 -Wno-suggest-attribute=format
	endif
	CFLAGS+= -Wcast-align -Wdisabled-optimization -Winit-self -Winline \
			 -Winvalid-pch -Wmissing-format-attribute -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector \
			 -Wswitch-default -Wvariadic-macros
else
	CFLAGS+= 
	# c2x for omitting parameter names in a function definition
	# gnu designator: plan9 extension: struct dicks = {[enum1] {..}, [enum2] {..}}
	ifeq ($(CC), clang)
		CFLAGS+= -Wno-c2x-extensions
	else
		CFLAGS+= -Wno-discarded-qualifiers
	endif
	CFLAGS+= -Wno-incompatible-pointer-types -Wno-ignored-qualifiers \
			 -Wno-unused-result -Wno-unused-function -Wno-unused-value \
			 -Wno-format-nonliteral
endif

LDFLAGS?=
LDLIBS?=

OBJ:=\
	sokol/impl_nuklear.o\
	sokol/impl_sokol_gfx.o\
	sokol/impl_sokol_nuklear.o\
	lib/chan.o\
	lib/queue.o\
	lib/HandmadeMath.o\
	lib/plan9/getfields.o\
	lib/plan9/seprint.o\
	lib/plan9/strecpy.o\
	posix/fs.o\
	posix/sys.o\
	posix/threads.o\
	cmd/awk.o\
	cmd/awkprog.o\
	cmd/cmd.o\
	draw/color.o\
	draw/draw.o\
	fs/em.o\
	fs/fs.o\
	fs/gfa.o\
	fs/layout.o\
	fs/load.o\
	fs/metacsv.o\
	graph/graph.o\
	layout/circ.o\
	layout/fr.o\
	layout/layout.o\
	layout/linear.o\
	layout/pfr.o\
	sokol/draw.o\
	sokol/impl_glsl.o\
	sokol/shaders.o\
	sokol/ui.o\
	ui/ui.o\
	util/print.o\
	strpg.o\
	strawk/awkgram.tab.o\
	strawk/b.o\
	strawk/parse.o\
	strawk/proctab.o\
	strawk/tran.o\
	strawk/lib.o\
	strawk/run.o\
	strawk/lex.o\
	strawk/mt19937-64.o\

GLSL:= $(patsubst %.glsl,%.h,$(wildcard glsl/*.glsl))

ifeq ($(TARGET),Unix)
	CPPFLAGS+= -Iunix -DSOKOL_GLCORE
	LDLIBS+= -lGL -lX11 -lXcursor -lXi -lm
	ifeq ($(OS),OpenBSD)
		CPPFLAGS+= -I/usr/X11R6/include
		LDFLAGS+= -L/usr/X11R6/lib
		LDLIBS+= -pthread
	else ifeq ($(OS),Cygwin)
		CFLAGS+= -mwin32
		LDLIBS+= -lkernel32 -luser32 -lshell32 -lgdi32 -lopengl32
	endif

else ifeq ($(TARGET),Win64)
	# FIXME
	CC:= x86_64-w64-mingw32-gcc-posix
	CPPFLAGS+= -Iwin64 -DSOKOL_D3D11
	LDFLAGS+= -static
	LDLIBS+= -lkernel32 -luser32 -lshell32 -ldxgi -ld3d11 -lole32 -lgdi32 -Wl,-Bstatic -lpthread
	OBJ+=\
		win64/stubs.o\

else ifeq ($(TARGET),MacOS)
	# FIXME
	# -target arm64-apple-macos15.1
	#CC:= x86_64-apple-darwin24-cc
	CPPFLAGS+= -Imacos -DSOKOL_METAL -D_DARWIN_C_SOURCE
	CFLAGS+= -ObjC
	LDFLAGS+=\
		-ObjC\
		-framework Cocoa\
		-framework QuartzCore\
		-framework Metal\
		-framework MetalKit\
		-dead_strip\
		-lpthread\

else
	$(error unknown target)
endif

ifdef EGL
	CPPFLAGS+= -DSOKOL_FORCE_EGL
	LDLIBS+= -lEGL
endif

DEPS:=$(patsubst %.o,%.d,$(OBJ))
CLEANFILES:= $(OBJ) $(DEPS)

all:	$(GLSL) $(ALLTARGETS) dirall

%.h: %.glsl
	sokol-shdc -f sokol_impl -i $^ -l glsl430:hlsl5:metal_macos:glsl300es -o $@

$(OBJ): $(GLSL)

$(BINTARGET):	$(OBJ)
	$(CC) $^ -o $@ $(LDLIBS) $(LDFLAGS)

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
		$(MAKE) uninstall; \
		cd ..; \
	done

dirall:
	for i in $(DIRS); do \
		cd $$i; \
		$(MAKE) $(MAKEFLAGS) all; \
		cd ..; \
	done

dirinstall:
	for i in $(DIRS); do \
		cd $$i; \
		$(MAKE) $(MAKEFLAGS) PREFIX=$(PREFIX) install; \
		cd ..; \
	done

clean:
	$(RM) $(CLEANFILES)
	for i in $(DIRS); do \
		cd $$i; \
		$(MAKE) clean; \
		cd ..; \
	done

-include $(DEPS)
