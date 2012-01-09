PREFIX=../rootfs
TARGET=wsa4000_cli

SRCS=main.c wsa4k_cli.c wsa_commons.c wsa_debug.c wsa_api.c wsa_lib.c \
     wsa_client.c

OBJS=$(SRCS:.c=.o)

CROSS=
CC=gcc
LD=ld
AR=ar

VERSION=$(shell git describe --dirty='+' --long)

LIBPREFIX=..
INCLUDE=.
LIBPATH=.
CFLAGS=-Wall -O2 -I${INCLUDE} -fPIC -DCLI_VERSION="\"${VERSION}\"" -DDEBUGLEVEL="\"${4}\""
LIBS=
LDFLAGS=-L${LIBPATH}

all: ${TARGET}

${TARGET}: ${OBJS}
	${CROSS}${CC} ${LDFLAGS} -o ${TARGET} ${OBJS} ${LIBS}
	${CROSS}strip ${TARGET}
	
${OBJS}:%.o:%.c
	${CROSS}${CC} ${CFLAGS} -c $<

install: ${TARGET}
	install -D ${TARGET} ${PREFIX}/bin/${TARGET}

doc: latex/Makefile ${TARGET}
	make -C latex

latex/Makefile: Doxyfile
	doxygen Doxyfile

clean:
	rm -f ${TARGET} ${OBJS}
	