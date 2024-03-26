ALLTARGETS:= $(patsubst %.c,%,$(wildcard *.c))
OBJS:=\
	../cmd/cmd.o\
	../graph/graph.o\
	../fs/fs.o\
	../fs/em.o\
	../fs/index.o\
	../fs/meta.o\
	../lib/chan.o\
	../lib/queue.o\
	../lib/plan9/nrand.o\
	../lib/plan9/getfields.o\
	../lib/plan9/seprint.o\
	../lib/plan9/strecpy.o\
	../linux/awk.o\
	../linux/fs.o\
	../linux/sys.o\
	../util/print.o\

#ALLOBJS:=$(OBJS)

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
	-I..\
	-I../draw\
	-I../fs\
	-I../graph\
	-I../index\
	-I../layout\
	-I../linux\
	-I../rend\
	-I../ui\
	-I../util\

CFLAGS+= $(SFLAGS) $(IFLAGS) $(WFLAGS)
LDFLAGS?=
LDLIBS?= -lGL -lglfw -lm

ifdef DEBUG
	WFLAGS+= -Waggregate-return -Wcast-align -Wcast-qual                \
			 -Wdisabled-optimization -Wfloat-equal -Winit-self -Winline \
			 -Winvalid-pch -Wunsafe-loop-optimizations                  \
			 -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector               \
			 -Wsuggest-attribute=const -Wswitch-default -Wunused        \
			 -Wvariadic-macros
	CFLAGS+= -g -glldb -O0
else
	# c2x for omitting parameter names in a function definition
	# gnu designator: plan9 extension: struct dicks = {[enum1] {..}, [enum2] {..}}
	WFLAGS+= -Wno-unknown-pragmas -Wno-unused-value \
			-Wno-unused-function -Wno-unused-parameter \
			-Wno-unused-variable -Wno-sign-compare \
			-Wno-ignored-qualifiers \
			-Wno-c2x-extensions -Wno-gnu-designator \
			-Wno-incompatible-pointer-types-discards-qualifiers \
			-Wno-format-nonliteral
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

CFLAGS+= $(OBJS)

all:	$(ALLTARGETS)

$(ALLTARGETS):	$(ALLOBJS)

clean:
	rm -f $(ALLOBJS) $(ALLTARGETS)
