PROGRAM:= strpg
BINTARGET:= $(PROGRAM)
INSTALLPREFIX:= /usr
BINDIR:= $(INSTALLPREFIX)/bin

OBJS:=\
	lib/chan.o\
	lib/queue.o\
	lib/plan9/geom.o\
	lib/plan9/getfields.o\
	lib/plan9/nrand.o\
	linux/fs.o\
	linux/layout.o\
	linux/sys.o\
	sokol/flextgl/flextGL.o\
	sokol/draw.o\
	draw/color.o\
	draw/draw.o\
	fs/fs.o\
	fs/gfa.o\
	fs/index.o\
	fs/load.o\
	graph/graph.o\
	graph/vertex.o\
	layout/conga.o\
	layout/force.o\
	layout/layout.o\
	layout/random.o\
	rend/rend.o\
	ui/ui.o\
	util/htab.o\
	util/print.o\
	strpg.o\

COARSEN2OBJS=\
	fs/fs.o\
	lib/plan9/getfields.o\
	linux/fs.o\
	linux/sys.o\
	n/coarsen2.o\
	util/print.o\

CC= clang
OFLAGS?= -O2 -pipe -march=native
CFLAGS?= $(OFLAGS)
# doesn't even work, what bullshit
CFLAGS+= -fextended-identifiers -finput-charset=UTF-8
# _XOPEN_SOURCE: M_PI et al
# _POSIX_C_SOURCE >= 200809L: getline
CFLAGS+= -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=500
CFLAGS+= -pthread
WFLAGS?= -Wall -Wextra -Wformat=2 -Wno-parentheses
SFLAGS?= -std=c99
IFLAGS?=\
	-I.\
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
	WFLAGS+= -Waggregate-return -Wcast-align -Wcast-qual                \
			 -Wdisabled-optimization -Wfloat-equal -Winit-self -Winline \
			 -Winvalid-pch -Wunsafe-loop-optimizations                  \
			 -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector               \
			 -Wsuggest-attribute=const -Wswitch-default -Wunused        \
			 -Wvariadic-macros
	CFLAGS+= -g -O0
else
	WFLAGS+= -Wno-unknown-pragmas -Wno-unused-value \
			-Wno-unused-function -Wno-unused-parameter \
			-Wno-unused-variable -Wno-sign-compare \
			-Wno-discarded-qualifiers
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

all:	$(BINTARGET) coarsen2

prepare:
	mkdir -p $(O)

$(BINTARGET):	$(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS) $(LDFLAGS)

coarsen2:	$(COARSEN2OBJS)
	$(CC) $(COARSEN2OBJS) -o $@ $(LDLIBS) $(LDFLAGS)

install:
	install -d -m755 $(BINDIR)
	install -m755 $(BINTARGET) $(BINDIR)

uninstall:
	rm $(BINDIR)/$(PROGRAM)

clean:
	rm -f $(OBJS) $(BINTARGET)

distclean:
	rm -rf $(O)
