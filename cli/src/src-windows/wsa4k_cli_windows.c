#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <sys/timeb.h>
#include <direct.h>

#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"


void get_current_time(TIME_HOLDER* current_time)
{
	_ftime64_s(current_time);
}

double get_time_difference(TIME_HOLDER* start_time, TIME_HOLDER* end_time)
{
	double result = 0;
	short delta_ms = 0;

	result = _difftime64(end_time->time, start_time->time);
	delta_ms = end_time->millitm - start_time->millitm;
	// Convert the milliseconds value to double, and add to result
	result += ( (double) delta_ms / (double) 1000);

	return result;
}

void generate_file_name(char* file_name, const char* prefix, const char* extension)
{
	TIME_HOLDER current_time;
	struct tm time_struct;
	char time_string[50];
	
	get_current_time(&current_time);
	_localtime64_s(&time_struct, &(current_time.time));
	strftime(time_string, sizeof(time_string), "%Y-%m-%d_%H%M%S", &time_struct);

	sprintf(file_name, "CAPTURES\\%s%s%03hu.%s", prefix,
		time_string,
		current_time.millitm,
		extension);
}

void open_captures_directory(void)
{
	char dir[500];	// be generous b/c overflow will kill ur program.

	sprintf(dir, "explorer %s\\CAPTURES", _getcwd(NULL, 0));

	// this using is a problem here for gcc/linux
	if (system(dir))
		printf("Open the folder of captured file(s)...\n");
	else 
		printf("Open failed!\n");
}

void print_captures_directory(void)
{
	printf("File directory is: \"%s\\CAPTURES\\\"\n", 
		_getcwd(NULL, 0));
}
