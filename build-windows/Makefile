CFLAGS = /Wall /W3 /D_CRT_SECURE_NO_WARNINGS

all : init executable

init : 
	if not exist lib\ ( mkdir lib )
	if not exist bin\ ( mkdir bin )

wsa_api.obj : ..\api\src\wsa_api.c
	cl /c $(CFLAGS) /I ..\api\include ..\api\src\wsa_api.c

wsa_lib.obj : ..\api\src\wsa_lib.c
	cl /c $(CFLAGS) /I ..\api\include ..\api\src\wsa_lib.c

wsa_commons.obj : ..\api\src\wsa_commons.c
	cl /c $(CFLAGS) /I ..\api\include ..\api\src\wsa_commons.c

wsa_debug.obj : ..\api\src\wsa_debug.c
	cl /c $(CFLAGS) /I ..\api\include ..\api\src\wsa_debug.c

wsa_client.obj : ..\api\src\wsa_client.c
	cl /c $(CFLAGS) /I ..\api\include /I ..\api\include-windows ..\api\src\wsa_client.c

wsa_client_windows.obj : ..\api\src-windows\wsa_client_windows.c
	cl /c $(CFLAGS) /I ..\api\include ..\api\src-windows\wsa_client_windows.c

wsa4k_cli.obj : ..\cli\src\wsa4k_cli.c
	cl /c $(CFLAGS) /I ..\cli\include /I ..\cli\include-windows /I ..\api\include ..\cli\src\wsa4k_cli.c

wsa4k_cli_windows.obj : ..\cli\src-windows\wsa4k_cli_windows.c
	cl /c $(CFLAGS) /I ..\cli\include /I ..\cli\include-windows /I ..\api\include ..\cli\src-windows\wsa4k_cli_windows.c

main.obj : ..\cli\src\main.c
	cl /c $(CFLAGS) /I ..\cli\include /I ..\cli\include-windows /I ..\api\include ..\cli\src\main.c

lib\wsa.lib : wsa_api.obj wsa_lib.obj wsa_commons.obj wsa_debug.obj wsa_client.obj wsa_client_windows.obj
	lib /OUT:lib\wsa.lib wsa_api.obj wsa_lib.obj wsa_commons.obj wsa_debug.obj wsa_client.obj wsa_client_windows.obj

executable : lib\wsa.lib wsa4k_cli.obj wsa4k_cli_windows.obj main.obj
	link /OUT:bin\wsa.exe main.obj wsa4k_cli.obj wsa4k_cli_windows.obj lib\wsa.lib WS2_32.Lib

clean : 
	if exist *.obj ( del /q *.obj )
	if exist bin\ ( rmdir /q /s bin )
	if exist lib\ ( rmdir /q /s lib )
