#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "wsa4k_cli_os_specific.h"
#include "wsa4k_cli.h"


void get_current_time(TIME_HOLDER* current_time)
{
	clock_gettime(CLOCK_REALTIME, current_time);
}

double get_time_difference(TIME_HOLDER* start_time, TIME_HOLDER* end_time)
{
	double result = 0;
	long delta_ns = 0;

	result = difftime(end_time->tv_sec, start_time->tv_sec);
	delta_ns = end_time->tv_nsec - start_time->tv_nsec;
	// Convert the nanoseconds value to double, and add to result
	result += ( (double) delta_ns / (double) 1000000000);

	return result;
}

void generate_file_name(char* file_name, const char* prefix, const char* extension)
{
	TIME_HOLDER current_time;
	struct tm* time_struct;
	char time_string[50];
	
	get_current_time(&current_time);
	time_struct = localtime(&(current_time.tv_sec));
	strftime(time_string, sizeof(time_string), "%Y-%m-%d_%H%M%S", time_struct);

	sprintf(file_name, "CAPTURES/%s%s%03ld.%s", prefix,
		time_string,
		(current_time.tv_nsec / 1000000), /* Convert nanoseconds to milliseconds */
		extension);
}

void open_captures_directory()
{
	char dir[500];	// be generous b/c overflow will kill ur program.
	DIR *temp;

	sprintf(dir, "%s/CAPTURES", getcwd(NULL, 0));

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
	printf("File directory is: \"%s/CAPTURES/\"\n", 
		getcwd(NULL, 0));
}
