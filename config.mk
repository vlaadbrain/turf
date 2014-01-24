# surf version
VERSION = 0.6

# Customize below to fit your system

# paths
PREFIX = ${HOME}
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

GTKINC = `pkg-config --cflags gmodule-2.0 gtk+-2.0 gthread-2.0 gtkglext-1.0`
GTKLIB = `pkg-config --libs gmodule-2.0 gtk+-2.0 gthread-2.0 gtkglext-1.0`

CEFINC = /home/cjt/cef3
CEFLIB = ${CEFINC}/Release

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} ${GTKINC} -I${CEFINC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 ${GTKLIB} -L${CEFLIB} -lcef -lffmpegsumo

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_BSD_SOURCE
CFLAGS = -std=c99 -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS = -g ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
