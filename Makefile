all : init executable

init : 
	if not exist api\build\ ( mkdir api\build )
	if not exist api\build\lib\ ( mkdir api\build\lib )
	if not exist cli\build\ ( mkdir cli\build )
	if not exist cli\build\bin\ ( mkdir cli\build\bin )

api\build\wsa_api.obj : api\src\wsa_api.c
	cl /c /I api\include /Foapi\build\ api\src\wsa_api.c

api\build\wsa_lib.obj : api\src\wsa_lib.c
	cl /c /I api\include /Foapi\build\ api\src\wsa_lib.c

api\build\wsa_commons.obj : api\src\wsa_commons.c
	cl /c /I api\include /Foapi\build\ api\src\wsa_commons.c

api\build\wsa_debug.obj : api\src\wsa_debug.c
	cl /c /I api\include /Foapi\build\ api\src\wsa_debug.c

api\build\wsa_client.obj : api\src\wsa_client.c
	cl /c /I api\include /I api\include-windows /Foapi\build\ api\src\wsa_client.c

api\build\wsa_client_windows.obj : api\src-windows\wsa_client_windows.c
	cl /c /I api\include /Foapi\build\ api\src-windows\wsa_client_windows.c

cli\build\wsa4k_cli.obj : cli\src\wsa4k_cli.c
	cl /c /I cli\include /I api\include /Focli\build\ cli\src\wsa4k_cli.c

cli\build\main.obj : cli\src\main.c
	cl /c /I cli\include /I api\include /Focli\build\ cli\src\main.c

api\build\lib\wsa.lib : api\build\wsa_api.obj api\build\wsa_lib.obj api\build\wsa_commons.obj api\build\wsa_debug.obj api\build\wsa_client.obj api\build\wsa_client_windows.obj
	lib /OUT:api\build\lib\wsa.lib api\build\wsa_api.obj api\build\wsa_lib.obj api\build\wsa_commons.obj api\build\wsa_debug.obj api\build\wsa_client.obj api\build\wsa_client_windows.obj

executable : api\build\lib\wsa.lib cli\build\wsa4k_cli.obj cli\build\main.obj
	link /OUT:cli\build\bin\wsa.exe cli\build\main.obj cli\build\wsa4k_cli.obj api\build\lib\wsa.lib WS2_32.Lib

clean : 
	rmdir /q /s api\build
	rmdir /q /s cli\build
