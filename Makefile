PROGRAM:= strangepg
VERSION:= 0.9.2
DIRS:=

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

ARCH?= $(shell uname -m)
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
	else ifeq ($(OS),Cygwin)
		TARGET:= Win64
	else
		TARGET:= Unix
	endif
endif

CC?= clang
CPPFLAGS+= -MMD -MP
CPPFLAGS+= -fextended-identifiers -finput-charset=UTF-8
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
# such as omitting parameter names in function definitions
# gnu designator: plan9 extension: struct dicks = {[enum1] {..}, [enum2] {..}}
CFLAGS+= -std=c11
CFLAGS+= -Wall -Wformat=2 -Wunused -Wno-parentheses -Wno-unknown-pragmas
ifdef DEBUG
	ifdef ASAN
		CFLAGS+= -O1 -fsanitize=address -fsanitize=leak -fsanitize=undefined
		CFLAGS+= -fsanitize-address-use-after-scope
		LDFLAGS+= -fsanitize=address -fsanitize=leak -fsanitize=undefined
		LDFLAGS+= -fsanitize-address-use-after-scope
		CPPFLAGS+= -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS -fexceptions
	else
		CFLAGS+= -O0
	endif
	CFLAGS+= -DSOKOL_DEBUG
	CFLAGS+= -fasynchronous-unwind-tables
	CFLAGS+= -fstack-clash-protection -fstack-protector-strong
	ifeq ($(CC), clang)
		export LLVM_PROFILE_FILE:= ./llvm_%p.prof
		CFLAGS+= -glldb -fprofile-instr-generate -fcoverage-mapping \
				 -Wno-c2x-extensions -Wno-cast-align
	else
		CFLAGS+= -ggdb -Wno-suggest-attribute=format -Wno-inline
	endif
	CFLAGS+= -Wdisabled-optimization -Winit-self \
			 -Winvalid-pch -Wmissing-format-attribute -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector \
			 -Wvariadic-macros
else
	#CPPFLAGS+= -DNDEBUG
	CFLAGS+= -g -flto
	LDFLAGS+= -flto
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

GLOBJ:=\
	sokol/draw.o\
	sokol/impl_glsl.o\
	sokol/shaders.o\
	sokol/ui.o\

OBJ:=\
	sokol/impl_nuklear.o\
	sokol/impl_sokol_gfx.o\
	sokol/impl_sokol_nuklear.o\
	lib/chan.o\
	lib/queue.o\
	lib/plan9/getfields.o\
	lib/plan9/seprint.o\
	lib/plan9/strecpy.o\
	posix/fs.o\
	posix/sys.o\
	posix/threads.o\
	cmd/awk.o\
	cmd/awkext.o\
	cmd/awkprog.o\
	cmd/cmd.o\
	cmd/var.o\
	draw/color.o\
	draw/draw.o\
	draw/view.o\
	fs/ctab.o\
	fs/em.o\
	fs/fs.o\
	fs/gfa.o\
	fs/layout.o\
	fs/load.o\
	fs/metacsv.o\
	fs/svg.o\
	graph/coarse.o\
	graph/graph.o\
	layout/layout.o\
	layout/pfr.o\
	ui/ui.o\
	util/print.o\
	util/rand.o\
	strpg.o\
	strawk/alloc.o\
	strawk/awkgram.tab.o\
	strawk/b.o\
	strawk/parse.o\
	strawk/proctab.o\
	strawk/tran.o\
	strawk/lib.o\
	strawk/run.o\
	strawk/lex.o\
	strawk/mt19937-64.o\
	strawk/unix.o\
	$(GLOBJ) \

GLSL:= $(patsubst %.glsl,%.h,$(wildcard glsl/*.glsl))

ifeq ($(TARGET),Unix)
	PREFIX?= $(HOME)/.local
	# _XOPEN_SOURCE: M_PI et al
	# _POSIX_C_SOURCE >= 200809L: getline (in _DEFAULT_SOURCE)
	CPPFLAGS+= -pthread -D_XOPEN_SOURCE=500
	CPPFLAGS+= -Iunix
	LDLIBS+= -lm
	LDLIBS+= -lX11 -lXcursor -lXi
	ifdef GLES
		CPPFLAGS+= -DSOKOL_GLES3
		LDLIBS+= -lGLESv2
		EGL:=1
	else
		CPPFLAGS+= -DSOKOL_GLCORE
		LDLIBS+= -lGL
	endif
	ifdef EGL
		CPPFLAGS+= -DSOKOL_FORCE_EGL
		LDLIBS+= -lEGL
	endif
	ifeq ($(OS),OpenBSD)
		CPPFLAGS+= -I/usr/X11R6/include
		LDFLAGS+= -L/usr/X11R6/lib
		LDLIBS+= -pthread
	endif
else ifeq ($(TARGET),Win64)
	PROGRAM:= $(PROGRAM).exe
	CPPFLAGS+= -pthread -D_XOPEN_SOURCE=500
	CPPFLAGS+= -Iwin64
	CFLAGS+= -mwin32 -mwindows
	ifeq ($(OS),Cygwin)
		PREFIX?= /usr
		CPPFLAGS+= -DSOKOL_GLCORE
		LDLIBS+= -lkernel32 -luser32 -lshell32 -lgdi32 -lopengl32
	else
		PREFIX?= ""
		# FIXME
		CC?= x86_64-w64-mingw32-gcc-posix
		CFLAGS+= -mthreads
		CPPFLAGS+= -DSOKOL_D3D11
		OBJ+=\
			win64/sys.o\

	endif
	LDFLAGS+= -static
	LDLIBS+= -lkernel32 -luser32 -lshell32 -ldxgi -ld3d11 -lole32 -lgdi32 -Wl,-Bstatic -lpthread

else ifeq ($(TARGET),MacOS)
	PREFIX?= /usr/local
	# FIXME: -target arm64-apple-macos15.1
	CPPFLAGS+= -pthread -D_DARWIN_C_SOURCE
	CPPFLAGS+= -Imacos -DSOKOL_METAL
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

ifneq ($(PREFIX),"")
	BINDIR:= $(PREFIX)/bin
else
	BINDIR:= .
endif
BINTARGET:= $(PROGRAM)
ALLTARGETS:=\
	$(BINTARGET)\

DEPS:=$(patsubst %.o,%.d,$(OBJ))
CLEANFILES:= $(OBJ) $(DEPS) $(BINTARGET)

all:	$(ALLTARGETS) dirall

# sucks, but we want to force creating or updating these headers before
# the source using them is built and avoid any compilation failure or
# accidental usage of old headers, without having to rebuild anything
# else on update
$(GLOBJ): $(GLSL)

glsl/%.h: glsl/%.glsl
	sokol-shdc -f sokol_impl -i $^ -l glsl430:hlsl5:metal_macos:glsl300es -o $@

$(BINTARGET):	$(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

install: $(ALLTARGETS) dirinstall
	test -d $(BINDIR) || install -d -m755 $(BINDIR)
	for i in $(ALLTARGETS); do \
		test -z $(PREFIX) || install -m755 $$i $(BINDIR); \
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
