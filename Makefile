PROGRAM:= strpg
#BINTARGET:= $(O)/$(PROGRAM)
BINTARGET:= $(PROGRAM)
INSTALLPREFIX:= /usr
BINDIR:= $(INSTALLPREFIX)/bin

OBJS:=\
	strpg.o\
	cmd/cmd.o\
	draw/draw.o\
	fs/fs.o\
	fs/gfa.o\
	fs/index.o\
	graph/graph.o\
	graph/vertex.o\
	layout/conga.o\
	layout/force.o\
	layout/layout.o\
	layout/random.o\
	linux/draw.o\
	linux/geom.o\
	linux/ui.o\
	linux/sys.o\
	rend/rend.o\
	ui/ui.o\
	util/htab.o\
	util/vec.o\

CC?= clang
OFLAGS?= -O2 -pipe -march=native
# doesn't even work, what bullshit
CFLAGS+=-fextended-identifiers -finput-charset=UTF-8
CFLAGS?= $(OFLAGS)
WFLAGS?= -Wall -Wextra -Wformat=2 -Wno-parentheses
SFLAGS?= -std=c11
IFLAGS?=\
	-I.\
	-Ilinux\
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
LDLIBS?= -lSDL2 -lSDL2_gfx -lm

ifdef DEBUG
	WFLAGS+= -Waggregate-return -Wcast-align -Wcast-qual                \
			 -Wdisabled-optimization -Wfloat-equal -Winit-self -Winline \
			 -Winvalid-pch -Wunsafe-loop-optimizations                  \
			 -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked \
			 -Wredundant-decls -Wshadow -Wstack-protector               \
			 -Wsuggest-attribute=const -Wswitch-default -Wunused        \
			 -Wvariadic-macros
	CFLAGS+= -g -O0
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

all:	$(BINTARGET)

prepare:
	mkdir -p $(O)

$(BINTARGET):	$(OBJS)
	$(CC) $(OBJS) -o $(BINTARGET) $(LDLIBS) $(LDFLAGS)

$(O)/%.o:	%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

install:
	install -d -m755 $(BINDIR)
	install -m755 $(BINTARGET) $(BINDIR)

uninstall:
	rm $(BINDIR)/$(PROGRAM)

clean:
	rm -f $(OBJS) $(BINTARGET)

distclean:
	rm -rf $(O)
