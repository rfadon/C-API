all : wsa.lib executable

wsa_api.obj : wsa_api.c
	cl /c wsa_api.c
	
wsa_lib.obj : wsa_lib.c
	cl /c wsa_lib.c
	
wsa_commons.obj : wsa_commons.c
	cl /c wsa_commons.c
	
wsa_debug.obj : wsa_debug.c
	cl /c wsa_debug.c
	
wsa_client.obj : wsa_client.c
	cl /c wsa_client.c
	
wsa_client_windows.obj : wsa_client_windows.c
	cl /c wsa_client_windows.c
	
wsa4k_cli.obj : wsa4k_cli.c
	cl /c wsa4k_cli.c
	
main.obj : main.c
	cl /c main.c
	
wsa.lib : wsa_api.obj wsa_lib.obj wsa_commons.obj wsa_debug.obj wsa_client.obj wsa_client_windows.obj
	lib /OUT:wsa.lib wsa_api.obj wsa_lib.obj wsa_commons.obj wsa_debug.obj wsa_client.obj wsa_client_windows.obj
	
executable : wsa.lib wsa4k_cli.obj main.obj
	link /OUT:wsa.exe main.obj wsa4k_cli.obj wsa.lib WS2_32.Lib
