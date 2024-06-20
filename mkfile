</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=\
	strpg\
	coarsen\

DIRS=\
	strawk\

OFILES=\
	strpg.$O\
	cmd/awkprog.$O\
	cmd/cmd.$O\
	draw/color.$O\
	draw/draw.$O\
	draw/shape.$O\
	fs/em.$O\
	fs/fs.$O\
	fs/gfa.$O\
	fs/index.$O\
	fs/load.$O\
	fs/meta.$O\
	fs/metacsv.$O\
	graph/graph.$O\
	layout/bo.$O\
	layout/circ.$O\
	layout/conga.$O\
	layout/fr.$O\
	layout/layout.$O\
	layout/linear.$O\
	layout/pfr.$O\
	layout/pline.$O\
	layout/random.$O\
	plan9/awk.$O\
	plan9/draw.$O\
	plan9/fs.$O\
	plan9/nanosec.$O\
	plan9/sys.$O\
	plan9/threads.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/print.$O\
	glsl/node.vert.$O\
	glsl/node.frag.$O\
	glsl/edge.vert.$O\
	glsl/edge.frag.$O\
	glsl/scr.vert.$O\
	glsl/scr.frag.$O\

OCOARSEN=\
	coarsen.$O\
	fs/em.$O\
	fs/fs.$O\
	plan9/fs.$O\
	plan9/nanosec.$O\
	plan9/sys.$O\
	plan9/threads.$O\
	util/print.$O\

HFILES=\
	dat.h\
	fns.h\
	cmd/cmd.h\
	draw/drw.h\
	fs/em.h\
	fs/fs.h\
	graph/graph.h\
	layout/layout.h\
	lib/khash.h\
	plan9/strpg.h\
	plan9/threads.h\
	ui/ui.h\
	util/dynar.h\

</sys/src/cmd/mkmany

CFLAGS=$CFLAGS -p -D__plan9__ -D__${objtype}__ \
	-I/sys/include/npe -Iplan9 \
	-I. -Icmd -Idraw -Ifs -Igraph -Ilayout -Irend -Iui -Iutil \

#LDFLAGS=$LDFLAGS -p

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c

$O.coarsen: $OCOARSEN
	$LD $LDFLAGS -o $target $prereq

cmd/awkprog.c:	cmd/main.awk
	n/awk2c.rc <$prereq >$target

glsl/%.c: glsl/%
	n/glsl2c.rc $prereq <$prereq >$target

CLEANFILES=$OFILES

all:V:	dirall
install:V:	dirinstall

dirinstall:V:
	for (i in $DIRS) @{
		cd $i
		mk install
	}

dirall:V:
	for (i in $DIRS) @{
		cd $i
		mk all
	}

clean:
	for(i in $DIRS test) @{
		cd $i
		mk clean
	}
	rm -f *.[$OS] *.a[$OS] y.tab.? lex.yy.c y.debug y.output [$OS].* $TARG $CLEANFILES

nuke:
	for(i in $DIRS test) @{
		cd $i
		mk nuke
	}
	rm -f *.[$OS] *.a[$OS] y.tab.? lex.yy.c y.debug y.output [$OS].* $TARG $CLEANFILES cmd/awkprog.c
