#include <stdio.h>
#include <Windows.h>
#include <sys/timeb.h>
#include <direct.h>

#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"

void get_current_time(TIME_HOLDER* msec_buf)
{
	_ftime_s(msec_buf);
}

void open_captures_directory()
{
	char dir[500];	// be generous b/c overflow will kill ur program.

	sprintf(dir, "explorer %s\\CAPTURES", _getcwd(NULL, 0));

	// this using is a problem here for gcc/linux
	if (system(dir))
		printf("Open the folder of captured file(s)...\n");
	else 
		printf("Open failed!\n");
}

void print_captures_directory()
{
	printf("File directory is: \"%s\\CAPTURES\\\"\n", 
		_getcwd(NULL, 0));
}
