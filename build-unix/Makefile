CFLAGS=-std=gnu89 -Wall -Wextra -Werror

all : init executable

init : 
	mkdir -p lib
	mkdir -p bin

wsa_api.o : ../api/src/wsa_api.c
	gcc -c $(CFLAGS) -I../api/include ../api/src/wsa_api.c

wsa_lib.o : ../api/src/wsa_lib.c
	gcc -c $(CFLAGS) -I../api/include ../api/src/wsa_lib.c

wsa_commons.o : ../api/src/wsa_commons.c
	gcc -c $(CFLAGS) -I../api/include ../api/src/wsa_commons.c

wsa_debug.o : ../api/src/wsa_debug.c
	gcc -c $(CFLAGS) -I../api/include ../api/src/wsa_debug.c

wsa_client.o : ../api/src/wsa_client.c
	gcc -c $(CFLAGS) -I../api/include -I../api/include-unix ../api/src/wsa_client.c

wsa_client_unix.o : ../api/src-unix/wsa_client_unix.c
	gcc -c $(CFLAGS) -I../api/include ../api/src-unix/wsa_client_unix.c

wsa4k_cli.o : ../cli/src/wsa4k_cli.c
	gcc -c $(CFLAGS) -I../cli/include -I../cli/include-unix -I../api/include ../cli/src/wsa4k_cli.c

wsa4k_cli_unix.o : ../cli/src-unix/wsa4k_cli_unix.c
	gcc -c $(CFLAGS) -I../cli/include -I../cli/include-unix -I../api/include ../cli/src-unix/wsa4k_cli_unix.c

main.o : ../cli/src/main.c
	gcc -c $(CFLAGS) -I../cli/include -I../cli/include-unix -I../api/include ../cli/src/main.c

lib/libwsa.a : wsa_api.o wsa_lib.o wsa_commons.o wsa_debug.o wsa_client.o wsa_client_unix.o
	ar -cvq lib/libwsa.a wsa_api.o wsa_lib.o wsa_commons.o wsa_debug.o wsa_client.o wsa_client_unix.o

executable : lib/libwsa.a wsa4k_cli.o wsa4k_cli_unix.o main.o
	gcc $(CFLAGS) -o bin/wsa.exe main.o wsa4k_cli.o wsa4k_cli_unix.o lib/libwsa.a -lm

clean : 
	-rm -f *.o
	-rm -fR bin
	-rm -fR lib
