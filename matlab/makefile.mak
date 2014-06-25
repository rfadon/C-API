#
#   Makefile to compile the libwsa64.a under windows.
#
#	nmake -f makefile.mak
#
#   Assumes C-API is located in ../api
#

WSFLAGS = -I..\api\include -I..\api\include\include-windows -O2

all:	wsa_api.obj wsa_client.obj wsa_client_windows.obj wsa_commons.obj wsa_debug.obj wsa_lib.obj wsa_probe.obj
	lib /out:libwsa64.a /MACHINE:X64 /NOLOGO wsa_api.obj wsa_client.obj wsa_client_windows.obj wsa_commons.obj wsa_debug.obj wsa_lib.obj wsa_probe.obj


wsa_api.obj:	..\api\src\wsa_api.c
	cl -c $(WSFLAGS) ..\api\src\wsa_api.c

wsa_client.obj: ..\api\src\wsa_client.c
	cl -c $(WSFLAGS) ..\api\src\wsa_client.c


wsa_commons.obj: ..\api\src\wsa_commons.c
	cl -c $(WSFLAGS) ..\api\src\wsa_commons.c


wsa_debug.obj: ..\api\src\wsa_debug.c
	cl -c $(WSFLAGS) ..\api\src\wsa_debug.c


wsa_lib.obj: ..\api\src\wsa_lib.c
	cl -c $(WSFLAGS) ..\api\src\wsa_lib.c


wsa_client_windows.obj: ..\api\src\src-windows\wsa_client_windows.c
	cl -c $(WSFLAGS) ..\api\src\src-windows\wsa_client_windows.c

wsa_probe.obj: ..\api\src\wsa_probe.c
	cl -c $(WSFLAGS) ..\api\src\wsa_probe.c

