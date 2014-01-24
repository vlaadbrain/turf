# turf - surf with cef 
# See LICENSE file for copyright and license details.

include config.mk

SRC = turf.c
OBJ = ${SRC:.c=.o}

all: options turf

options:
	@echo turf build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

turf: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ turf.o ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f turf ${OBJ}

.PHONY: all options clean
