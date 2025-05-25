</$objtype/mkfile

TARG=fontsrv
OFILES=\
	fontsrv.$O \
	freetype.$O \

HFILES=dat.h

CFLAGS=$CFLAGS -p -I./libfreetype/include
LIB=libfreetype/libfreetype.a$O

BIN=/$objtype/bin

</sys/src/cmd/mkone

$LIB:V:
	cd libfreetype
	mk install

install:V:	$BIN/$TARG /sys/lib/fontsrv /sys/man/4/fontsrv.4
/sys/lib/fontsrv:
	cp lib/fontsrv /sys/lib/fontsrv
/sys/man/4/fontsrv.4: man/4/fontsrv.4
	cp man/4/fontsrv.4 /sys/man/4/fontsrv.4

clean nuke:V:
	@{ cd libfreetype; mk $target }
	rm -f *.[$OS] [$OS].out $TARG

