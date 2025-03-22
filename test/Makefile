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
	-I../.\
	-I../cmd\
	-I../draw\
	-I../fs\
	-I../graph\
	-I../layout\
	-I../posix\
	-I../rend\
	-I../ui\
	-I../util\

CFLAGS?= -O3 -pipe -march=native
# with minor plan9esque violations
# such as omitting parameter names in function definitions
# gnu designator: plan9 extension: struct dicks = {[enum1] {..}, [enum2] {..}}
CFLAGS+= -std=c11
CFLAGS+= -Wall -Wformat=2 -Wunused -Wno-parentheses -Wno-unknown-pragmas
ifdef DEBUG
	ifdef ASAN
		CFLAGS+= -O1 -fsanitize=address -fsanitize=leak -fsanitize=undefined
		LDFLAGS+= -fsanitize=address -fsanitize=leak -fsanitize=undefined
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

ifeq ($(TARGET),Unix)
	PREFIX?= $(HOME)/.local
	# _XOPEN_SOURCE: M_PI et al
	# _POSIX_C_SOURCE >= 200809L: getline (in _DEFAULT_SOURCE)
	CPPFLAGS+= -pthread -D_XOPEN_SOURCE=500
	CPPFLAGS+= -I../unix
	LDLIBS+= -lm
	ifdef EGL
		CPPFLAGS+= -DSOKOL_FORCE_EGL
		LDLIBS+= -lEGL
	endif
	LDLIBS+= -lX11 -lXcursor -lXi
	ifeq ($(ARCH)$(GLCORE),aarch64)
		CPPFLAGS+= -DSOKOL_GLES3
		LDLIBS+= -lGLESv2
	else
		CPPFLAGS+= -DSOKOL_GLCORE
		LDLIBS+= -lGL
	endif
	ifeq ($(OS),OpenBSD)
		CPPFLAGS+= -I/usr/X11R6/include
		LDFLAGS+= -L/usr/X11R6/lib
		LDLIBS+= -pthread
	endif

else ifeq ($(TARGET),Win64)
	PROGRAM:= $(PROGRAM).exe
	PREFIX?= /usr/local
	CPPFLAGS+= -pthread -D_XOPEN_SOURCE=500
	CPPFLAGS+= -I../win64
	CFLAGS+= -mwin32 -mwindows
	ifeq ($(OS),Cygwin)
		CPPFLAGS+= -DSOKOL_GLCORE
		LDLIBS+= -lkernel32 -luser32 -lshell32 -lgdi32 -lopengl32
	else
		# FIXME
		CC?= x86_64-w64-mingw32-gcc-posix
		CFLAGS+= -mthreads
		CPPFLAGS+= -DSOKOL_D3D11
	endif
	LDFLAGS+= -static
	LDLIBS+= -lkernel32 -luser32 -lshell32 -ldxgi -ld3d11 -lole32 -lgdi32 -Wl,-Bstatic -lpthread
	COMMON+=\
		win64/sys.o\

else ifeq ($(TARGET),MacOS)
	PREFIX?= /usr/local
	# FIXME: -target arm64-apple-macos15.1
	CPPFLAGS+= -pthread -D_DARWIN_C_SOURCE
	CPPFLAGS+= -I../macos -DSOKOL_METAL
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

COMMON+=\
	stubs.o\
	../cmd/cmd.o\
	../graph/graph.o\
	../posix/sys.o\
	../util/print.o\

OBJ:=$(patsubst %.c,%.o,$(wildcard *.c))
DEPS:=$(patsubst %.o,%.d,$(OBJ) $(COMMON))

ALLTARGETS:=\
	fs_gfa

fs_gfa:	fs_gfa.o ../fs/fs.o ../posix/fs.o $(COMMON)
	echo $(ASS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

all:	$(ALLTARGETS)

clean:
	rm -f $(COMMON) $(OBJ) $(DEPS) $(ALLTARGETS)

-include $(DEPS)
