//*****************************************************************************
// Created by Jean Richard
//*****************************************************************************
 
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wsa_debug.h"

int doutf(int level, const char *fmt, ...)
{
	va_list ap;
	FILE* debugFile;
	time_t rawtime;
	struct tm * timeinfo;
	int n;

	// don't print if debug level is too low
	if (level > DEBUGLEVEL)
		return 0;
		
	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	fflush(stdout);

	if (ENABLE_LOG_FILE)
	{
		debugFile = fopen(WSA_API_LOG_FILE, "a");
	
		if (debugFile) {
			time(&rawtime);
			timeinfo = localtime(&rawtime);

			va_start(ap, fmt);
			fprintf(debugFile, "[%d-%02d-%02d %02d:%02d:%02d] [Level %d] ", 
				timeinfo->tm_year + 1900, 
				timeinfo->tm_mon + 1, 
				timeinfo->tm_mday, 
				timeinfo->tm_hour, 
				timeinfo->tm_min, 
				timeinfo->tm_sec,
				level);
			vfprintf(debugFile, fmt, ap);
			fclose(debugFile);
			va_end(ap);
		}
	}

	return n;
}
