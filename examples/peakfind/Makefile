CC=gcc
CFLAGS += -I../../include
LDFLAGS += -L../../build-unix-32/lib/
LIBS = -lwsa32 -lm

all: peakfind

peakfind: peakfind.o
	gcc $(LDFLAGS) peakfind.o -o peakfind $(LIBS)

clean:
	rm -f *.o peakfind
