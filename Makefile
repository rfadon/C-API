PREFIX=../rootfs
TARGET=wsa4000_cli

SRCS=main.c wsa4k_cli.c wsa_commons.c wsa_debug.c wsa_api.c wsa_lib.c \
     wsa_client.c

OBJS=$(SRCS:.c=.o)

CROSS=mb-linux-
CC=gcc
LD=ld
AR=ar

INCLUDE=
LIBPATH=
CFLAGS=-Wall -O2 -I${INCLUDE} -fPIC
LIBS=
LDFLAGS=-L${LIBPATH}

VERSION=$(shell git describe --dirty='+' --long)

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
