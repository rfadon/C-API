//*****************************************************************************
// Created by Jean Richard
//*****************************************************************************

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "wsa_debug.h"

int doutf(int level, const char *fmt, ...)
{
	va_list ap;
	int n;

	// don't print if debug level is too low
	if (level > DEBUGLEVEL)
		return 0;
		
	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	fflush(stdout);

	return n;
}