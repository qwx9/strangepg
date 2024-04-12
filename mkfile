</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=\
	strpg\
	coarsen\

DIRS=\
	strawk\

OFILES=\
	strpg.$O\
	cmd/cmd.$O\
	draw/color.$O\
	draw/draw.$O\
	fs/em.$O\
	fs/fs.$O\
	fs/gfa.$O\
	fs/index.$O\
	fs/load.$O\
	fs/meta.$O\
	graph/graph.$O\
	graph/vertex.$O\
	layout/conga.$O\
	layout/force.$O\
	layout/fr.$O\
	layout/layout.$O\
	layout/linear.$O\
	layout/pfr.$O\
	layout/pline.$O\
	layout/random.$O\
	rend/rend.$O\
	plan9/awk.$O\
	plan9/draw.$O\
	plan9/fs.$O\
	plan9/nanosec.$O\
	plan9/sys.$O\
	plan9/threads.$O\
	plan9/ui.$O\
	ui/ui.$O\
	util/print.$O\

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
	layout/layout.h\
	lib/khash.h\
	plan9/strpg.h\
	plan9/threads.h\
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

install:V:	/tmp/main.awk

/tmp/main.awk: cmd/main.awk
	cp $prereq $target

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
	rm -f *.[$OS] *.a[$OS] y.tab.? lex.yy.c y.debug y.output [$OS].* $TARG $CLEANFILES
