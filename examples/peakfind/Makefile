ARCH?=32
OS?=unix

CC=gcc
CFLAGS += -I../../include
LDFLAGS += -L../../build-$(OS)-$(ARCH)/lib/
LIBS = -lwsa32 -lm

all: peakfind

peakfind: peakfind.o ../../build-$(OS)-$(ARCH)/lib/libwsa$(ARCH).a
	gcc $(LDFLAGS) peakfind.o -o peakfind $(LIBS)

clean:
	rm -f *.o peakfind
