</$objtype/mkfile

TARG=fontsrv
OFILES=\
	fontsrv.$O \
	freetype.$O \

HFILES=dat.h
LIBDIR=libfreetype

CFLAGS=$CFLAGS -p -I$LIBDIR/builds/plan9 -I$LIBDIR/include
LIB=$LIBDIR/libfreetype.a$O

BIN=/$objtype/bin

</sys/src/cmd/mkone

$LIB:V:
	cd $LIBDIR; mk -f builds/plan9/mkfile install

install:V:	$BIN/$TARG /sys/lib/fontsrv.map /sys/man/4/fontsrv.4
/sys/lib/fontsrv.map:
	cp lib/fontsrv /sys/lib/fontsrv.map
/sys/man/4/fontsrv.4: man/4/fontsrv.4
	cp man/4/fontsrv.4 /sys/man/4/fontsrv.4

clean nuke:V:
	@{ cd $LIBDIR; mk -f builds/plan9/mkfile $target }
	rm -f *.[$OS] [$OS].out $TARG

