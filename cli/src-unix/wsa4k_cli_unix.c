#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"

void get_current_time(TIME_HOLDER* msec_buf)
{
	ftime(msec_buf);
}

void open_captures_directory()
{
	char dir[500];	// be generous b/c overflow will kill ur program.

	sprintf(dir, "%s\\CAPTURES", getcwd(NULL, 0));

	temp = opendir(dir);
	if (temp == NULL) {
		if (errno == EACCES)
			printf("Error: Permission to open '%s is denied'.\n", dir);
		else if (errno == ENOENT)
			printf("Error: '%s' directory does not exist.\n", dir);
	}
	else 
		printf("Open the folder of captured file(s)...\n");
}

void print_captures_directory()
{
	printf("File directory is: \"%s\\CAPTURES\\\"\n", 
		getcwd(NULL, 0));
}